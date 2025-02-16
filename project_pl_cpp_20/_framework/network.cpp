#include "./network.h"

#include "./connector.h"
#include "./buffer.h"
#include "./packetDataQueue.h"
#include "../_lib/util.h"
#include "../_lib/exceptionReport.h"
#include "../_lib/log.h"

#include <iostream>
#include <string>
#include <format>
#include <source_location>

//
Network::Network(void)
{
}

Network::~Network(void)
{
	Finalize();
}

bool Network::Initialize()
{
	bool bRet = false;

	//
	WSADATA wsa;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsa)) {
		return false;
	}

	_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (NULL == _hIOCP) {
		LogError("NetworkHandle create fail");
		return false;
	}

	// thread 초기화
	for (int idx = 0; idx < _config.workerThreadCount; ++idx) {
		_threads.emplace_back(thread{ &Network::WorkerThread, this, _threadStop.get_token()});
	}

	if (_config.doListen) {
		_threads.emplace_back(thread{ &Network::AcceptThread, this, _threadStop.get_token() });
	}

	{
		_threads.emplace_back(thread{ &Network::UpdateThread, this, _threadStop.get_token() });
	}

	return true;
}

bool Network::Finalize()
{
	WSACleanup();

	return true;
}

bool Network::Start()
{
	_threadSuspended = 0;
	_threadSuspended.notify_all();

	return true;
}

bool Network::Stop()
{
	_threadStop.request_stop();

	//
	for (int nIndex = 0; nIndex < _config.workerThreadCount; ++nIndex) {
		PostQueuedCompletionStatus(_hIOCP, 0, 0, NULL);
	}

	if (INVALID_SOCKET != _ListenSock) {
		closesocket(_ListenSock);
		_ListenSock = INVALID_SOCKET;
	}

	for (auto& t : _threads) {
		t.join();
	}

	//
	return true;
}

unsigned int Network::AcceptThread(stop_token token)
{
	Log(format("log: {} create", source_location::current().function_name()));
	_threadSuspended.wait(1);

	//
	CConnectorMgr &ConnectMgr = CConnectorMgr::GetInstance();
	CConnector* pConnector = nullptr;

	HANDLE &hNetworkHandle = _hIOCP;
	NetworkConfig& config = _config;

	INT32 iWSARet = 0;
	HANDLE hRet = INVALID_HANDLE_VALUE;
	BOOL bRet = FALSE;

	DWORD dwRecvDataSize = 0;
	DWORD dwFlag = 0;

	//
	SOCKET listenSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == listenSock) {
		Log("error: WSASocket fail.");
		return 1;
	}

	SOCKADDR_IN listenAddr = { 0, };
	memset(&listenAddr, 0, sizeof(listenAddr));
	listenAddr.sin_family = AF_INET;
	listenAddr.sin_addr.s_addr = /*inet_addr(pszListenIP);*/ htonl(INADDR_ANY);
	listenAddr.sin_port = htons(_config.listenInfo.wPort);

	//
	if (SOCKET_ERROR == ::bind(listenSock, (SOCKADDR*)&listenAddr, sizeof(listenAddr))) {
		int nErrorCode = WSAGetLastError();
		LogError(format("bind fail. error {}", nErrorCode));
		return false;
	}
	if (SOCKET_ERROR == listen(listenSock, 10)) {
		int nErrorCode = WSAGetLastError();
		LogError(format("listen fail. error {}", nErrorCode));
		return false;
	}
	Log(format("log: accept bind port: {}", _config.listenInfo.wPort));

	_ListenSock = listenSock;
	_ListenAddr = listenAddr;

	//
	Log(format("log: {}: start", source_location::current().function_name()));
	while( !token.stop_requested() ) {
		SOCKADDR_IN AcceptAddr;
		memset(&AcceptAddr, 0, sizeof(AcceptAddr));
		int iAcceptAddrLen = sizeof(AcceptAddr);

		//
		SOCKET AcceptSock = WSAAccept(listenSock, (SOCKADDR*)&AcceptAddr, &iAcceptAddrLen, NULL, NULL);
		if( INVALID_SOCKET == AcceptSock ) {
			int nErrorCode = WSAGetLastError();
			LogError(format("accept fail. error{}", nErrorCode));
			continue;
		}

		BOOL bSockOpt = TRUE;
		bSockOpt = TRUE;
		setsockopt(AcceptSock, SOL_SOCKET, SO_DONTLINGER, (char*)&bSockOpt, sizeof(BOOL));
		bSockOpt = TRUE;
		setsockopt(AcceptSock, IPPROTO_TCP, TCP_NODELAY, (char*)&bSockOpt, sizeof(BOOL));

		//
		pConnector = ConnectMgr.GetFreeConnector(); // new CConnector;
		if( !pConnector ) {
			LogError(format("connector create fail. socket: {}", AcceptSock));
			closesocket(AcceptSock);
			continue;
		} else {
			//Log(format("<{}> connector create. socket:{}", pConnector->GetIndex(), AcceptSock));
		}

		//
		pConnector->SetSocket(AcceptSock);
		memcpy((void*)&pConnector->_SockAddr, (void*)&AcceptAddr, sizeof(SOCKADDR_IN));

		pConnector->ConvertSocket2IP();

		//
		hRet = CreateIoCompletionPort((HANDLE)AcceptSock, hNetworkHandle, (ULONG_PTR)pConnector, 0);
		if( hNetworkHandle != hRet )
		{
			int nErrorCode = GetLastError();
			LogError(format("<{}> CreateIoCompletionPort() fail. socket:{}. error:{}", pConnector->GetUID(), AcceptSock, nErrorCode));
			closesocket(AcceptSock);
			ConnectMgr.ReleaseConnector(pConnector); //SAFE_DELETE(pConnector);
			continue;
		}

		//pConnector->SetActive();

		//
		if( 0 >= pConnector->RecvPrepare() )
		{
			LogError(format("<{}> RecvPrepare() fail. socket:{}", pConnector->GetUID(), AcceptSock));
			closesocket(AcceptSock);
			ConnectMgr.ReleaseConnector(pConnector); //SAFE_DELETE(pSession);
			continue;
		}

		//
		dwFlag = 0;
		iWSARet = WSARecv(pConnector->GetSocket(), pConnector->GetRecvWSABuffer(), 1, &dwRecvDataSize, &dwFlag, pConnector->GetRecvOverlapped(), NULL);
		if( SOCKET_ERROR == iWSARet )
		{
			int nErrorCode = WSAGetLastError();
			if( WSA_IO_PENDING != nErrorCode )
			{
				LogError(format("<{}> WSARecv fail. socket:{}. error:{}", pConnector->GetUID(), AcceptSock, nErrorCode));
				closesocket(AcceptSock);
				ConnectMgr.ReleaseConnector(pConnector); //SAFE_DELETE(pSession);
			}
		}
	}

	Log(format("log: {}: end", source_location::current().function_name()));
	return 1;
}

unsigned int Network::WorkerThread(stop_token token)
{
	Log(format("log: {} create", source_location::current().function_name()));
	_threadSuspended.wait(1);

	CConnectorMgr &ConnectMgr = CConnectorMgr::GetInstance();
	CConnector* pConnector = nullptr;

	CRecvPacketQueue &RecvPacketQueue = CRecvPacketQueue::GetInstance();

	//
	OVERLAPPED *lpOverlapped = nullptr;
	OVERLAPPED_EX *lpOverlappedEx = nullptr;
	int nSessionIndex = 0;

	DWORD dwIOSize = 0;
	DWORD dwFlags = 0;

	BOOL bRet = FALSE;
	int iRet = 0;
	int nErrorCode = 0;

	CNetworkBuffer *pNetworkBuffer = nullptr;

	DWORD dwSendDataSize = 0;
	DWORD dwRecvDataSize = 0;

	HANDLE &hNetworkHandle = _hIOCP;

	//
	Log(format("log: {}: start", source_location::current().function_name())); 
	while( !token.stop_requested() )
	{
		dwIOSize = 0;
		bRet = FALSE;

		bRet = GetQueuedCompletionStatus(hNetworkHandle, &dwIOSize, (PULONG_PTR)&pConnector, &lpOverlapped, 500);
		if( FALSE == bRet )
		{
			nErrorCode = WSAGetLastError();
			if (WAIT_TIMEOUT == nErrorCode) {
				continue;
			}
			Log(format("log: WorkerThread(): GetQueuedCompletionStatus() result false. lpOverlapped {:x}. error:{}", reinterpret_cast<intptr_t>(lpOverlapped), nErrorCode));

			//
			if (NULL == lpOverlapped) {
				continue;
			}
			
			if( pConnector ) {
				Disconnect(pConnector);
			}

			continue;
		}

		//
		if( NULL == lpOverlapped ) {
			Log("error: WorkerThread(): lpOverlapped is NULL");
			continue;
		}
		lpOverlappedEx = (OVERLAPPED_EX*)lpOverlapped;

		//
		pConnector = lpOverlappedEx->pSession;
		if( !pConnector ) {
			Log(format("error: WorkerThread(): Invalid connector {:x}", reinterpret_cast<intptr_t>(pConnector)));
			continue;
		}

		pNetworkBuffer = lpOverlappedEx->pBuffer;
		if( !pNetworkBuffer ) {
			Log(format("error: WorkerThread() : Invalid NetworkBuffer {:x}", reinterpret_cast<intptr_t>(pNetworkBuffer)));
			continue;
		}

		// disconnect
		if( 0 == dwIOSize ) {
			if( pConnector ) {
				Log(FormatW(L"log: WorkerThread() : <%d> Disconnect. IP %s, Socket %d", pConnector->GetIndex(), pConnector->GetDomain(), pConnector->GetSocket()));

				if( INVALID_SOCKET != pConnector->GetSocket() ) {
					shutdown(pConnector->GetSocket(), SD_BOTH);
					closesocket(pConnector->GetSocket());
				}

				//
				if( pConnector->GetSendRef() || pConnector->GetRecvRef() || pConnector->GetInnerRef() ) {
					switch( pNetworkBuffer->GetOperator() ) {
						case eNetworkBuffer::OP_SEND: pConnector->DecSendRef(); break;
						case eNetworkBuffer::OP_RECV: pConnector->DecRecvRef(); break;
						case eNetworkBuffer::OP_INNER: pConnector->DecInnerRef(); break;
					}
				}
				
				if( 0 >= pConnector->GetSendRef() || 0 >= pConnector->GetRecvRef() || 0 >= pConnector->GetInnerRef() ) {
					ConnectMgr.ReleaseConnector(pConnector); //SAFE_DELETE(pConnector);
				}
			}
			else {
				Log(format("error: WorkerThread() : unknown connector. IoSize {}, pCurrSockEx {:x}", dwIOSize, reinterpret_cast<intptr_t>(lpOverlappedEx)));
			}

			continue;
		}

		//
		switch( pNetworkBuffer->GetOperator() ) {
			case eNetworkBuffer::OP_SEND:
				{
					Log(format("debug: WorkerThread() : <{}> SendResult : socket:{}. SendRequestSize {}, SendSize {}", pConnector->GetIndex(), pConnector->GetSocket(), (int)pNetworkBuffer->_nDataSize, dwIOSize));
					PacketLog(format(L"debug: WorkerThread(): <{}> SendData:", pConnector->GetIndex()), pNetworkBuffer->_pBuffer, dwIOSize);

					// 전송 완료
					DWORD dwRemainData = pConnector->SendComplete(dwIOSize);

					//
					if( 0 < dwRemainData || 0 < pConnector->SendPrepare() ) {
						// 계속 전송
						iRet = WSASend(pConnector->GetSocket(), pConnector->GetSendWSABuffer(), 1, &dwSendDataSize, 0, pConnector->GetSendOverlapped(), NULL);
						if( SOCKET_ERROR == iRet ) {
							nErrorCode = WSAGetLastError();
							if( WSA_IO_PENDING != nErrorCode ) {
								Log(format("error: WorkerThread() : <{}> WSASend() fail. socket:{}. error:{}", pConnector->GetIndex(), pConnector->GetSocket(), nErrorCode));
								Disconnect(pConnector);
							}
						}
					}
				}
				break;
			case eNetworkBuffer::OP_RECV:
				{
					Log(format("debug: WorkerThread(): <{}> RecvResult : socket:{}. RecvDataSize {}", pConnector->GetIndex(), pConnector->GetSocket(), dwIOSize));
					PacketLog(format(L"debug: WorkerThread(): <{}> RecvData: ", pConnector->GetIndex()), pNetworkBuffer->_pBuffer, dwIOSize);

					// 수신 완료
					int nResult = pConnector->RecvComplete(dwIOSize);
					if( 0 > nResult ) {
						Log(format("error: WorkerThread(): <{}> CConnector::RecvComplete() fail. socket:{}", pConnector->GetIndex(), pConnector->GetSocket()));
						Disconnect(pConnector);
					} else {
						if( 0 < pConnector->RecvPrepare() ) {
							// 계속 수신
							iRet = WSARecv(pConnector->GetSocket(), pConnector->GetRecvWSABuffer(), 1, &dwRecvDataSize, &dwFlags, pConnector->GetRecvOverlapped(), NULL);
							if( SOCKET_ERROR == iRet ) {
								nErrorCode = WSAGetLastError();
								if( WSA_IO_PENDING != nErrorCode ) {
									Log(format("error: WorkerThread(): <{}> WSARecv() fail. socket:{}. error:{}", pConnector->GetIndex(), pConnector->GetSocket(), nErrorCode));
									Disconnect(pConnector);
								}
							}
						} else {
							Log(format("error: WorkerThread(): <{}> CConnector::RecvPrepare() fail. socket:{}", pConnector->GetIndex(), pConnector->GetSocket()));
						}
					}
				}
				break;
			case eNetworkBuffer::OP_INNER:
				{
					Log(format("debug: WorkerThread() : <{}> InnerResult : socket:{}. RecvDataSize {}", pConnector->GetIndex(), pConnector->GetSocket(), dwIOSize));
					PacketLog(format(L"debug: WorkerThread(): <{}> InnerData: ", pConnector->GetIndex()), pNetworkBuffer->_pBuffer, dwIOSize);

					// 내부 수신처리
					pConnector->InnerComplete(dwIOSize);

					// 내부 전송처리
					int nRemainData = pConnector->InnerPrepare();
					if( 0 < nRemainData ) {
						BOOL bRet = PostQueuedCompletionStatus(hNetworkHandle, nRemainData, (ULONG_PTR)pConnector, pConnector->GetInnerOverlapped());
						if( 0 == bRet ) {
							nErrorCode = WSAGetLastError();
							Log(format("error: WorkerThread(): <{}> PostQueuedCompletionStatus() fail. socket:{}. error:{}", pConnector->GetIndex(), pConnector->GetSocket(), nErrorCode));
							Disconnect(pConnector);
						}
					}
				}
				break;
			default:
				{
					// 뭐여?
				}
				break;
		}
	}

	Log(format("log: {}: end", source_location::current().function_name()));
	return 0;
}

unsigned int Network::UpdateThread(stop_token token)
{
	_threadSuspended.wait(1);

	//
	CConnectorMgr &ConnectorMgr = CConnectorMgr::GetInstance();
	CConnector* pConnector = nullptr;

	//
	void *pParam = nullptr;

	INT64 biCurrTime = 0;
	INT64 biReleaseCheck = GetTimeMilliSec() + (MILLISEC_A_SEC * 30);

	std::list<CConnector*> ReleaseConnectorList {};

	//
	Log(format("log: {}: start", source_location::current().function_name()));
	while( !token.stop_requested() ) {
		biCurrTime = GetTimeMilliSec();

		{
			//SafeLock lock(ConnectorMgr._lock);
			//for( auto pConnector : ConnectorMgr._usedList )
			//{
			//	if( pConnector->DoUpdate(biCurrTime) )
			//	{
			//		if( false == pConnector->GetActive() && false == pConnector->GetUsed() )
			//		{
			//			ReleaseConnectorList.push_back(pConnector);
			//			continue;
			//		}
			//	}

			//	pConnector->CheckHeartbeat(biCurrTime);
			//}
		}

		{
			//if( false == ReleaseConnectorList.empty() )
			//{
			//	for( auto pConnector : ReleaseConnectorList )
			//	{
			//		ConnectorMgr.ReleaseConnector(pConnector);
			//	}

			//	ReleaseConnectorList.clear();
			//}
		}

		{
			DoUpdate(biCurrTime);
		}

		//
		this_thread::sleep_for(1ms);
	}

	Log(format("log: {}: end", source_location::current().function_name()));
	return 0;
}

bool Network::lookup_host(const char *hostname, std::string &hostIP)
{
	ADDRINFO hints, *result = nullptr;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags |= AI_CANONNAME;

	char szDomain[eNetwork::MAX_LEN_DOMAIN_STRING + 1] = {0,};
	strncpy_s(szDomain, eNetwork::MAX_LEN_DOMAIN_STRING, hostname, eNetwork::MAX_LEN_DOMAIN_STRING);

	int ret = getaddrinfo(szDomain, NULL, &hints, &result);
	if( 0 == ret )
	{
		char addrstr[100] {0,};
		void* ptr{ nullptr };
		inet_ntop(result->ai_family, result->ai_addr->sa_data, addrstr, _countof(addrstr));
		switch( result->ai_family ) {
		case AF_INET:
			ptr = &((sockaddr_in*)result->ai_addr)->sin_addr;
			break;
		case AF_INET6:
			ptr = &((sockaddr_in6*)result->ai_addr)->sin6_addr;
			break;
		}
		inet_ntop(result->ai_family, ptr, addrstr, 100);
		//SockAddr.sin_addr.s_addr = *(unsigned long*)addrstr;
		//InetPtonA(AF_INET, (char*)addrstr, &SockAddr.sin_addr);

		hostIP = (char*)addrstr;
		return true;
	}
	else
	{
		return false;
	}
}

CConnector* Network::Connect(WCHAR *pwcsDomain, const WORD wPort)
{
	CConnector *pConn = CConnectorMgr::GetInstance().GetFreeConnector(); //new CConnector;
	if (!pConn) {
		return nullptr;
	}
	bool bActive = true;

	//
	pConn->SetDomain(pwcsDomain, wPort);

	//
	SOCKADDR_IN SockAddr;
	memset((void*)&SockAddr, 0, sizeof(SockAddr));

	BOOL bSockOpt = TRUE;
	int nRet = 0;

	//
	SockAddr.sin_family = AF_INET;
	if( iswalpha(pwcsDomain[0]) ) {
		//struct hostent *host = gethostbyname(pConn->GetDomainA());
		//if( NULL == host )
		//{
		//	goto ProcFail;
		//}
		//SockAddr.sin_addr.s_addr = *(unsigned long*)host->h_addr_list[0];

		string strHostIP{};
		bool bRet = lookup_host(pConn->GetDomainA(), strHostIP);
		if( bRet ) {
			InetPtonA(AF_INET, strHostIP.c_str(), &SockAddr.sin_addr);
		} else {
			LogError("GetAddrInfo fail");
			//failProc();
			//return nullptr;
		}
	} else {
		InetPtonW(AF_INET, pwcsDomain, &SockAddr.sin_addr);
	}

	SockAddr.sin_port = htons(wPort);

	// 소켓 생성
	SOCKET sock = pConn->SetSocket(WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED));
	if( INVALID_SOCKET == pConn->GetSocket() ) {
		LogError("WSASocket() fail");
		CConnectorMgr::GetInstance().ReleaseConnector(pConn);
		return nullptr;
	} else {
		bSockOpt = TRUE;
		setsockopt(sock, SOL_SOCKET, SO_DONTLINGER, (char*)&bSockOpt, sizeof(BOOL));
		bSockOpt = TRUE;
		setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&bSockOpt, sizeof(BOOL));
	}

	//
	// 연결 시도
	nRet = connect(sock, (SOCKADDR*)&SockAddr, sizeof(SockAddr));
	if (SOCKET_ERROR == nRet) {
		int nErrorCode = WSAGetLastError();
		LogError(format("connect() fail.error:{}", nErrorCode));
		CConnectorMgr::GetInstance().ReleaseConnector(pConn);
		return nullptr;
	}
	memcpy((void*)&pConn->_SockAddr, (void*)&SockAddr, sizeof(SockAddr));

	//Log(format("log: Network::Connect(): <{}> Domain: {}, Port: {}, Socket: {}", pConn->GetUID(), pwcsDomain, wPort, sock));

	//
	HANDLE hRet = CreateIoCompletionPort((HANDLE)sock, _hIOCP, (ULONG_PTR)pConn, 0);
	if(_hIOCP != hRet ) {
		CConnectorMgr::GetInstance().ReleaseConnector(pConn);
		return nullptr;
	}

	// 데이터 수신 대기
	DWORD dwRecvDataSize = 0;
	if( 0 >= pConn->RecvPrepare() ) {
		Log(format("<{}> RecvPrepare() fail", pConn->GetUID()));
		CConnectorMgr::GetInstance().ReleaseConnector(pConn);
		return nullptr;
	}

	//
	DWORD dwFlags = 0;
	nRet = WSARecv(pConn->GetSocket(), pConn->GetRecvWSABuffer(), 1, &dwRecvDataSize, &dwFlags, pConn->GetRecvOverlapped(), NULL);
	if( SOCKET_ERROR == nRet ) {
		int nErrorCode = WSAGetLastError();
		if( WSA_IO_PENDING != nErrorCode ) {
			Log(format("error: Network::Connect(): WSARecv() fail. {}, Socket {}. error:{}", pConn->GetUID(), pConn->GetSocket(), nErrorCode));
			CConnectorMgr::GetInstance().ReleaseConnector(pConn);
			return nullptr;
		}
	}

	//
	return pConn;
}

bool Network::Disconnect(CConnector *pConnector)
{
	if( !pConnector )
	{
		LogError(format("Invalid session object {:x}", reinterpret_cast<intptr_t>(pConnector)));
		return false;
	}

	//pConnector->SetDeactive();

	//
	Log(format("info: <{}> Disconnect. socket {}", pConnector->GetUID(), pConnector->GetSocket()));

	//
	if( INVALID_SOCKET != pConnector->GetSocket() )
	{
		shutdown(pConnector->GetSocket(), SD_BOTH);
		closesocket(pConnector->GetSocket());

		pConnector->SetSocket(INVALID_SOCKET);
	}

	//
	if( pConnector->GetRecvRef() ) PostQueuedCompletionStatus(_hIOCP, 0, (ULONG_PTR)pConnector, pConnector->GetRecvOverlapped());

	//
	return true;
}

bool Network::Disconnect(SOCKET socket)
{
	if( INVALID_SOCKET == socket )
		return false;

	//
	shutdown(socket, SD_BOTH);
	closesocket(socket);

	//
	return true;
}

eResultCode Network::Write(CConnector *pConnector, char *pSendData, int iSendDataSize)
{
	if( !pConnector )
	{
		//LogError(format("Invalid session object %08X %d", pConnector, (pConnector ? pConnector->GetUID() : -1)));
		return eResultCode::RESULT_FAIL;
	}
	if( INVALID_SOCKET == pConnector->GetSocket() )
	{
		LogError(format("<{}> Invalid socket {}", pConnector->GetUID(), pConnector->GetSocket()));
		return eResultCode::RESULT_SOCKET_DISCONNECTED;
	}

	// 전송데이터 셋팅
	if( 0 > pConnector->AddSendQueue(pSendData, iSendDataSize) )
	{
		LogError(format("<{}> Invalid send data {}", pConnector->GetUID(), pConnector->GetSocket()));
		return eResultCode::RESULT_FAIL;
	}

	//
	if( 0 >= pConnector->GetSendRef() )
	{
		// 지금 전송
		if( 0 > pConnector->SendPrepare() )
		{
			LogError(format("<{}> SendPrepare() fail", pConnector->GetUID()));
			return eResultCode::RESULT_FAIL;
		}

		//
		DWORD dwSendDataSize = 0;
		DWORD dwFlags = 0;
		int nRet = WSASend(pConnector->GetSocket(), pConnector->GetSendWSABuffer(), 1, &dwSendDataSize, dwFlags, pConnector->GetSendOverlapped(), NULL);
		if( SOCKET_ERROR == nRet )
		{
			int nErrorCode = WSAGetLastError();
			if( WSA_IO_PENDING != nErrorCode )
			{
				LogError(format("<{}> WSASend() fail. socket:{}. error:{}", pConnector->GetUID(), pConnector->GetSocket(), nErrorCode));
				return eResultCode::RESULT_FAIL;
			}
		}
	}

	//
	return eResultCode::RESULT_SUCC;
}

eResultCode Network::InnerWrite(CConnector *pConnector, char *pSendData, int nSendDataSize)
{
	if( !pConnector ) {
		//LogError(format("Invalid session object {:x} {}", reinterpret_cast<intptr_t>(pConnector), (pConnector ? pConnector->GetUID() : -1)));
		eResultCode::RESULT_FAIL;
	}
	if( INVALID_SOCKET == pConnector->GetSocket() ) {
		LogError(format("<{}> Invalid socket {}", pConnector->GetUID(), pConnector->GetSocket()));
		eResultCode::RESULT_FAIL;
	}

	// 데이터 셋팅
	pConnector->AddInnerQueue(pSendData, nSendDataSize);

	//
	if( 0 >= pConnector->GetInnerRef() )
	{
		// 지금처리
		if( 0 > pConnector->InnerPrepare() ) {
			LogError(format("<{}> InnerPrepare() fail", pConnector->GetUID()));
			return eResultCode::RESULT_FAIL;
		}

		BOOL bRet = PostQueuedCompletionStatus(_hIOCP, nSendDataSize, (ULONG_PTR)pConnector, pConnector->GetInnerOverlapped());
		if( 0 == bRet ) {
			int nErrorCode = GetLastError();
			LogError(format("<{}> PostQueuedCompletionStatus() fail. socket:{}. error:{}", pConnector->GetUID(), pConnector->GetSocket(), nErrorCode));
			return eResultCode::RESULT_FAIL;
		}
	}

	//
	return eResultCode::RESULT_SUCC;
}

//