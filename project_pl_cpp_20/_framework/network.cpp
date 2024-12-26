#include "./network.h"

#include "./connector.h"
#include "./buffer.h"
#include "./packetDataQueue.h"
#include "./util.h"
#include "./exceptionReport.h"

#include <string>
#include <iostream>

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
		WriteMiniNetLog(L"error: MiniNet::Init(): NetworkHandle create fail");
		return false;
	}

	// thread 초기화
	unsigned int uiThreadID = 0;
	for (int iIndex = 0; iIndex < _config.workerThreadCount; ++iIndex) {
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, NULL, CREATE_SUSPENDED, &uiThreadID);
		if (NULL == hThread) {
			WriteMiniNetLog(L"error: Network::Init(): WorkerThread create fail");
			return false;
		}

		_threadHandleSet.insert(hThread);
	}

	if (_config.doListen) {
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, AcceptThread, NULL, CREATE_SUSPENDED, &uiThreadID);
		if (NULL == hThread) {
			WriteMiniNetLog(L"error: Network::Listen(): Accept thread create fail");
			return false;
		}
		_threadHandleSet.insert(hThread);
	}

	{
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, UpdateThread, NULL, CREATE_SUSPENDED, &uiThreadID);
		if (NULL == hThread) {
			WriteMiniNetLog(L"error: Network::Init(): UpdateThread create fail");
			return false;
		}
		_threadHandleSet.insert(hThread);
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
	InterlockedExchange((LONG*)&_dwRunning, 1);

	for (HANDLE hThread : _threadHandleSet) {
		ResumeThread(hThread);
	}

	return true;
}

bool Network::Stop()
{
	InterlockedExchange((LONG*)&_dwRunning, 0);

	//
	for (int nIndex = 0; nIndex < _config.workerThreadCount; ++nIndex) {
		PostQueuedCompletionStatus(_hIOCP, 0, 0, NULL);
	}

	for (HANDLE hThread : _threadHandleSet) {
		WaitForSingleObject(hThread, INFINITE);
	}

	if (INVALID_SOCKET != _ListenSock) {
		closesocket(_ListenSock);
		_ListenSock = INVALID_SOCKET;
	}

	//
	return true;
}

unsigned int WINAPI Network::AcceptThread(void *p)
{
	Network &net = Network::GetInstance();
	CConnectorMgr &ConnectMgr = CConnectorMgr::GetInstance();

	//
	DWORD& dwRunning = net._dwRunning;

	HANDLE &hNetworkHandle = net._hIOCP;

	NetworkConfig& config = net._config;

	CConnector *pConnector = nullptr;

	INT32 iWSARet = 0;
	HANDLE hRet = INVALID_HANDLE_VALUE;
	BOOL bRet = FALSE;

	DWORD dwRecvDataSize = 0;
	DWORD dwFlag = 0;

	//
	SOCKET listenSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == listenSock) {
		WriteMiniNetLog(L"error: MiniNet::AcceptThread(): WSASocket fail.");
		return 1;
	}

	SOCKADDR_IN listenAddr = { 0, };
	memset(&listenAddr, 0, sizeof(listenAddr));
	listenAddr.sin_family = AF_INET;
	listenAddr.sin_addr.s_addr = /*inet_addr(pszListenIP);*/ htonl(INADDR_ANY);
	listenAddr.sin_port = htons(net._config.listenInfo.wPort);

	//
	if (SOCKET_ERROR == ::bind(listenSock, (SOCKADDR*)&listenAddr, sizeof(listenAddr))) {
		int nErrorCode = WSAGetLastError();
		WriteMiniNetLog(FormatW(L"error: MiniNet::AcceptThread(): bind fail. error %d", nErrorCode));
		return false;
	}
	if (SOCKET_ERROR == listen(listenSock, 10)) {
		int nErrorCode = WSAGetLastError();
		WriteMiniNetLog(FormatW(L"error: MiniNet::AcceptThread(): listen fail. error %d", nErrorCode));
		return false;
	}

	net._ListenSock = listenSock;
	net._ListenAddr = listenAddr;

	//
	WriteMiniNetLog(L"log: MiniNet::AcceptThread() : start");
	//g_Log.Write(L"log: MiniNet::AcceptThread() : start");


	while( 1 == InterlockedExchange((long*)&dwRunning, dwRunning) )
	{
		SOCKADDR_IN AcceptAddr;
		memset(&AcceptAddr, 0, sizeof(AcceptAddr));
		int iAcceptAddrLen = sizeof(AcceptAddr);

		//
		SOCKET AcceptSock = WSAAccept(listenSock, (SOCKADDR*)&AcceptAddr, &iAcceptAddrLen, NULL, NULL);
		if( INVALID_SOCKET == AcceptSock )
		{
			int nErrorCode = WSAGetLastError();
			WriteMiniNetLog(FormatW(L"error: MiniNet::AcceptThread() : accept() fail. error:%d", nErrorCode));
			continue;
		}

		BOOL bSockOpt = TRUE;
		bSockOpt = TRUE;
		setsockopt(AcceptSock, SOL_SOCKET, SO_DONTLINGER, (char*)&bSockOpt, sizeof(BOOL));
		bSockOpt = TRUE;
		setsockopt(AcceptSock, IPPROTO_TCP, TCP_NODELAY, (char*)&bSockOpt, sizeof(BOOL));

		//
		pConnector = ConnectMgr.GetFreeConnector(); // new CConnector;
		if( !pConnector )
		{
			WriteMiniNetLog(FormatW(L"error: MiniNet::AcceptThread() : connector create fail. socket:%d", AcceptSock));
			closesocket(AcceptSock);
			continue;
		}
		else
		{
			WriteMiniNetLog(FormatW(L"log: MiniNet::AcceptThread() : <%d> connector create. socket:%d", pConnector->GetIndex(), AcceptSock));
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
			WriteMiniNetLog(FormatW(L"error: MiniNet::AcceptThread() : <%d> CreateIoCompletionPort() fail. socket:%d. error:%d", pConnector->GetIndex(), AcceptSock, nErrorCode));
			closesocket(AcceptSock);
			ConnectMgr.ReleaseConnector(pConnector); //SAFE_DELETE(pConnector);
			continue;
		}

		//pConnector->SetActive();

		//
		if( 0 >= pConnector->RecvPrepare() )
		{
			WriteMiniNetLog(FormatW(L"error: MiniNet::AcceptThread() : <%d> RecvPrepare() fail. socket:%d", pConnector->GetIndex(), AcceptSock));
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
				WriteMiniNetLog(FormatW(L"error: MiniNet::AcceptThread() : <%d> WSARecv fail. socket:%d. error:%d", pConnector->GetIndex(), AcceptSock, nErrorCode));
				closesocket(AcceptSock);
				ConnectMgr.ReleaseConnector(pConnector); //SAFE_DELETE(pSession);
			}
		}
	}

	return 1;
}

unsigned int WINAPI Network::WorkerThread(void *p)
{
	Network &Net = Network::GetInstance();
	CConnectorMgr &ConnectMgr = CConnectorMgr::GetInstance();
	CRecvPacketQueue &RecvPacketQueue = CRecvPacketQueue::GetInstance();

	//
	DWORD &dwRunning = Net._dwRunning;

	OVERLAPPED *lpOverlapped = nullptr;
	OVERLAPPED_EX *lpOverlappedEx = nullptr;
	int nSessionIndex = 0;

	DWORD dwIOSize = 0;
	DWORD dwFlags = 0;

	BOOL bRet = FALSE;
	int iRet = 0;
	int nErrorCode = 0;

	CConnector *pConnector = nullptr;
	CNetworkBuffer *pNetworkBuffer = nullptr;

	DWORD dwSendDataSize = 0;
	DWORD dwRecvDataSize = 0;

	HANDLE &hNetworkHandle = Net._hIOCP;

	//
	WriteMiniNetLog(L"log: MiniNet::WorkerThread() : start"); 

	while( 1 == InterlockedExchange((long*)&dwRunning, dwRunning) )
	{
		dwIOSize = 0;
		bRet = FALSE;

		bRet = GetQueuedCompletionStatus(hNetworkHandle, &dwIOSize, (PULONG_PTR)&pConnector, &lpOverlapped, INFINITE);
		if( FALSE == bRet )
		{
			nErrorCode = WSAGetLastError();
			if( WAIT_TIMEOUT == nErrorCode )
				continue;

			//
			WriteMiniNetLog(FormatW(L"log: MiniNet::WorkerThread() : GetQueuedCompletionStatus() result false. lpOverlapped %08X. error:%d", lpOverlapped, nErrorCode));

			//
			if( NULL == lpOverlapped )
				continue;
			
			//if( ERROR_NETNAME_DELETED == nErrorCode )
			{
				if( pConnector )
				{
					Net.Disconnect(pConnector);
				}
			}

			continue;
		}

		//
		if( NULL == lpOverlapped )
		{
			WriteMiniNetLog(L"error: MiniNet::WorkerThread() : lpOverlapped is NULL");
			continue;
		}

		//
		lpOverlappedEx = (OVERLAPPED_EX*)lpOverlapped;

		pConnector = lpOverlappedEx->pSession;
		if( !pConnector )
		{
			WriteMiniNetLog(FormatW(L"error: MiniNet::WorkerThread() : Invalid connector %08X", pConnector));
			continue;
		}

		pNetworkBuffer = lpOverlappedEx->pBuffer;
		if( !pNetworkBuffer )
		{
			WriteMiniNetLog(FormatW(L"error: MiniNet::WorkerThread() : Invalid NetworkBuffer %08X", pNetworkBuffer));
			continue;
		}

		//
		if( 0 == dwIOSize )
		{
			//
			if( pConnector )
			{
				WriteMiniNetLog(FormatW(L"log: MiniNet::WorkerThread() : <%d> Disconnect. IP %s, Socket %d", pConnector->GetIndex(), pConnector->GetDomain(), pConnector->GetSocket()));

				if( INVALID_SOCKET != pConnector->GetSocket() )
				{
					shutdown(pConnector->GetSocket(), SD_BOTH);
					closesocket(pConnector->GetSocket());
				}

				//
				if( pConnector->GetSendRef() || pConnector->GetRecvRef() || pConnector->GetInnerRef() )
				{
					switch( pNetworkBuffer->GetOperator() )
					{
						case eNetworkBuffer::OP_SEND: pConnector->DecSendRef(); break;
						case eNetworkBuffer::OP_RECV: pConnector->DecRecvRef(); break;
						case eNetworkBuffer::OP_INNER: pConnector->DecInnerRef(); break;
					}
				}
				
				if( 0 >= pConnector->GetSendRef() || 0 >= pConnector->GetRecvRef() || 0 >= pConnector->GetInnerRef() )
				{
					ConnectMgr.ReleaseConnector(pConnector); //SAFE_DELETE(pConnector);
				}
			}
			else
			{
				WriteMiniNetLog(FormatW(L"error: MiniNet::WorkerThread() : <%d> IoSize %d, pCurrSockEx %08X", dwIOSize, lpOverlappedEx));
			}

			continue;
		}

		//
		switch( pNetworkBuffer->GetOperator() )
		{
		case eNetworkBuffer::OP_SEND:
			{
				WriteMiniNetLog(FormatW(L"debug: MiniNet::WorkerThread() : <%d> SendResult : socket:%d. SendRequestSize %d, SendSize %d", pConnector->GetIndex(), pConnector->GetSocket(), (int)pNetworkBuffer->_nDataSize, dwIOSize));
				WritePacketLog(FormatW(L"debug: MiniNet::WorkerThread(): <%d> SendData:", pConnector->GetIndex()), pNetworkBuffer->_pBuffer, dwIOSize);

				// 전송 완료
				DWORD dwRemainData = pConnector->SendComplete(dwIOSize);

				//
				if( 0 < dwRemainData || 0 < pConnector->SendPrepare() )
				{
					// 계속 전송
					iRet = WSASend(pConnector->GetSocket(), pConnector->GetSendWSABuffer(), 1, &dwSendDataSize, 0, pConnector->GetSendOverlapped(), NULL);
					if( SOCKET_ERROR == iRet )
					{
						nErrorCode = WSAGetLastError();
						if( WSA_IO_PENDING != nErrorCode )
						{
							WriteMiniNetLog(FormatW(L"error: MiniNet::WorkerThread() : <%d> WSASend() fail. socket:%d. error:%d", pConnector->GetIndex(), pConnector->GetSocket(), nErrorCode));
							Net.Disconnect(pConnector);
						}
					}
				}
			}
			break;
		case eNetworkBuffer::OP_RECV:
			{
				WriteMiniNetLog(FormatW(L"debug: MiniNet::WorkerThread(): <%d> RecvResult : socket:%d. RecvDataSize %d", pConnector->GetIndex(), pConnector->GetSocket(), dwIOSize));
				WritePacketLog(FormatW(L"debug: MiniNet::WorkerThread(): <%d> RecvData: ", pConnector->GetIndex()), pNetworkBuffer->_pBuffer, dwIOSize);

				// 수신 완료
				int nResult = pConnector->RecvComplete(dwIOSize);
				if( 0 > nResult )
				{
					WriteMiniNetLog(FormatW(L"error: MiniNet::WorkerThread(): <%d> CConnector::RecvComplete() fail. socket:%d", pConnector->GetIndex(), pConnector->GetSocket()));
					Net.Disconnect(pConnector);
				}
				else
				{
					//
					if( 0 < pConnector->RecvPrepare() )
					{
						// 계속 수신
						iRet = WSARecv(pConnector->GetSocket(), pConnector->GetRecvWSABuffer(), 1, &dwRecvDataSize, &dwFlags, pConnector->GetRecvOverlapped(), NULL);
						if( SOCKET_ERROR == iRet )
						{
							nErrorCode = WSAGetLastError();
							if( WSA_IO_PENDING != nErrorCode )
							{
								WriteMiniNetLog(FormatW(L"error: MiniNet::WorkerThread(): <%d> WSARecv() fail. socket:%d. error:%d", pConnector->GetIndex(), pConnector->GetSocket(), nErrorCode));
								Net.Disconnect(pConnector);
							}
						}
					}
					else
					{
						WriteMiniNetLog(FormatW(L"error: MiniNet::WorkerThread(): <%d> CConnector::RecvPrepare() fail. socket:%d", pConnector->GetIndex(), pConnector->GetSocket()));
					}
				}
			}
			break;
		case eNetworkBuffer::OP_INNER:
			{
				WriteMiniNetLog(FormatW(L"debug: MiniNet::WorkerThread() : <%d> InnerResult : socket:%d. RecvDataSize %d", pConnector->GetIndex(), pConnector->GetSocket(), dwIOSize));
				WritePacketLog(FormatW(L"debug: MiniNet::WorkerThread(): <%d> InnerData: ", pConnector->GetIndex()), pNetworkBuffer->_pBuffer, dwIOSize);

				// 내부 수신처리
				pConnector->InnerComplete(dwIOSize);

				// 내부 전송처리
				int nRemainData = pConnector->InnerPrepare();
				if( 0 < nRemainData )
				{
					BOOL bRet = PostQueuedCompletionStatus(hNetworkHandle, nRemainData, (ULONG_PTR)pConnector, pConnector->GetInnerOverlapped());
					if( 0 == bRet )
					{
						nErrorCode = WSAGetLastError();
						WriteMiniNetLog(FormatW(L"error: MiniNet::WorkerThread(): <%d> PostQueuedCompletionStatus() fail. socket:%d. error:%d", pConnector->GetIndex(), pConnector->GetSocket(), nErrorCode));
						Net.Disconnect(pConnector);
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

	//
	return 0;
}

unsigned int WINAPI Network::UpdateThread(void *p)
{
	Network &Net = Network::GetInstance();
	CConnectorMgr &ConnectorMgr = CConnectorMgr::GetInstance();

	//
	DWORD &dwRunning = Net._dwRunning;

	CConnector *pConnector = nullptr;
	void *pParam = nullptr;

	INT64 biCurrTime = 0;
	INT64 biReleaseCheck = GetTimeMilliSec() + (MILLISEC_A_SEC * 30);

	std::list<CConnector*> ReleaseConnectorList = {};

	//
	WriteMiniNetLog(L"log: MiniNet::UpdateThread() : start");

	while( 1 == InterlockedExchange((long*)&dwRunning, dwRunning) )
	{
		biCurrTime = GetTimeMilliSec();

		{
			ScopeLock lock(ConnectorMgr._Lock);
			for( auto pConnector : ConnectorMgr._UsedConnectorList )
			{
				if( pConnector->DoUpdate(biCurrTime) )
				{
					if( false == pConnector->GetActive() && false == pConnector->GetUsed() )
					{
						ReleaseConnectorList.push_back(pConnector);
						continue;
					}
				}

				pConnector->CheckHeartbeat(biCurrTime);
			}
		}

		{
			if( false == ReleaseConnectorList.empty() )
			{
				for( auto pConnector : ReleaseConnectorList )
				{
					ConnectorMgr.ReleaseConnector(pConnector);
				}

				ReleaseConnectorList.clear();
			}
		}

		{
			Net.DoUpdate(biCurrTime);
		}

		//
		Sleep(1);
	}

	//
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
		char addrstr[100] = {0,};
		void *ptr = nullptr;
		inet_ntop(result->ai_family, result->ai_addr->sa_data, addrstr, _countof(addrstr));
		switch( result->ai_family )
		{
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
	CConnector *pConnector = CConnectorMgr::GetInstance().GetFreeConnector(); //new CConnector;
	if( !pConnector )
		return nullptr;

	pConnector->SetDomain(pwcsDomain, wPort);

	//
	SOCKADDR_IN SockAddr;
	memset((void*)&SockAddr, 0, sizeof(SockAddr));

	BOOL bSockOpt = TRUE;
	int nRet = 0;

	SOCKET connectSocket = INVALID_SOCKET;

	auto failProc = [&]() -> void {
		if (INVALID_SOCKET != connectSocket) {
			closesocket(connectSocket);
		}
		CConnectorMgr::GetInstance().ReleaseConnector(pConnector);
	};

	//
	SockAddr.sin_family = AF_INET;

	if( iswalpha(pwcsDomain[0]) )
	{
		//struct hostent *host = gethostbyname(pConnector->GetDomainA());
		//if( NULL == host )
		//{
		//	goto ProcFail;
		//}
		//SockAddr.sin_addr.s_addr = *(unsigned long*)host->h_addr_list[0];

		string strHostIP = {};
		bool bRet = lookup_host(pConnector->GetDomainA(), strHostIP);
		if( bRet )
		{
			InetPtonA(AF_INET, strHostIP.c_str(), &SockAddr.sin_addr);
		}
		else
		{
			WriteMiniNetLog(L"error: Network::Connect(): GetAddrInfo fail");
			failProc();
			return nullptr;
		}
	}
	else
	{
		InetPtonW(AF_INET, pwcsDomain, &SockAddr.sin_addr);
	}

	SockAddr.sin_port = htons(wPort);

	// 소켓 생성
	connectSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if( INVALID_SOCKET == connectSocket )
	{
		WriteMiniNetLog(L"error: MiniNet::Connect(): WSASocket() fail");
		
	}

	bSockOpt = TRUE;
	setsockopt(connectSocket, SOL_SOCKET, SO_DONTLINGER, (char*)&bSockOpt, sizeof(BOOL));
	bSockOpt = TRUE;
	setsockopt(connectSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&bSockOpt, sizeof(BOOL));

	// 연결 시도
	nRet = connect(connectSocket, (SOCKADDR*)&SockAddr, sizeof(SockAddr));
	if( SOCKET_ERROR == nRet )
	{
		int nErrorCode = WSAGetLastError();
		WriteMiniNetLog(FormatW(L"error: MiniNet::Connect(): connect() fail. error:%d", nErrorCode));
		failProc();
		return nullptr;
	}

	WriteMiniNetLog(FormatW(L"log: MiniNet::Connect(): <%d> Domain: %s, Port: %d, Socket: %d", pConnector->GetIndex(), pwcsDomain, wPort, connectSocket));

	//
	pConnector->SetSocket(connectSocket);
	memcpy((void*)&pConnector->_SockAddr, (void*)&SockAddr, sizeof(SockAddr));

	//
	HANDLE hRet = CreateIoCompletionPort((HANDLE)connectSocket, _hIOCP, (ULONG_PTR)pConnector, 0);
	if(_hIOCP != hRet )
	{
		failProc();
		return nullptr;
	}

	// 데이터 수신 대기
	DWORD dwRecvDataSize = 0;

	if( 0 >= pConnector->RecvPrepare() )
	{
		WriteMiniNetLog(FormatW(L"error: MiniNet::Connect(): <%d> RecvPrepare() fail", pConnector->GetIndex()));
		failProc();
		return nullptr;
	}

	//
	DWORD dwFlags = 0;
	nRet = WSARecv(pConnector->GetSocket(), pConnector->GetRecvWSABuffer(), 1, &dwRecvDataSize, &dwFlags, pConnector->GetRecvOverlapped(), NULL);
	if( SOCKET_ERROR == nRet )
	{
		int nErrorCode = WSAGetLastError();
		if( WSA_IO_PENDING != nErrorCode )
		{
			WriteMiniNetLog(FormatW(L"error: MiniNet::Connect(): WSARecv() fail. %d, Socket %d. error:() %d", pConnector->GetIndex(), pConnector->GetSocket(), nErrorCode));
			failProc();
			return nullptr;
		}
	}

	//
	return pConnector;
}

bool Network::Disconnect(CConnector *pConnector)
{
	if( !pConnector )
	{
		WriteMiniNetLog(FormatW(L"error: MiniNet::Disconnect(): Invalid session object %08X", pConnector));
		return false;
	}

	//pConnector->SetDeactive();

	//
	WriteMiniNetLog(FormatW(L"log: MiniNet::Disconnect(): <%d> Disconnect. socket %d", pConnector->GetIndex(), pConnector->GetSocket()));

	//
	if( pConnector->GetSocket() )
	{
		shutdown(pConnector->GetSocket(), SD_BOTH);
		closesocket(pConnector->GetSocket());
	}

	pConnector->SetSocket(INVALID_SOCKET);

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
		WriteMiniNetLog(FormatW(L"error: MiniNet::Write(): Invalid session object %08X %d", pConnector, (pConnector ? pConnector->GetIndex() : -1)));
		return eResultCode::RESULT_FAIL;
	}
	if( INVALID_SOCKET == pConnector->GetSocket() )
	{
		WriteMiniNetLog(FormatW(L"error: MiniNet::Write(): <%d> Invalid socket %d", pConnector->GetIndex(), pConnector->GetSocket()));
		return eResultCode::RESULT_SOCKET_DISCONNECTED;
	}

	// 전송데이터 셋팅
	if( 0 > pConnector->AddSendQueue(pSendData, iSendDataSize) )
	{
		WriteMiniNetLog(FormatW(L"error: MiniNet::Write(): <%d> Invalid send data %d", pConnector->GetIndex(), pConnector->GetSocket()));
		return eResultCode::RESULT_FAIL;
	}

	//
	if( 0 >= pConnector->GetSendRef() )
	{
		// 지금 전송
		if( 0 > pConnector->SendPrepare() )
		{
			WriteMiniNetLog(FormatW(L"error: MiniNet::Write(): <%d> SendPrepare() fail", pConnector->GetIndex()));
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
				WriteMiniNetLog(FormatW(L"error: MiniNet::Write(): <%d> WSASend() fail. socket:%d. error:%d", pConnector->GetIndex(), pConnector->GetSocket(), nErrorCode));
				return eResultCode::RESULT_FAIL;
			}
		}
	}

	//
	return eResultCode::RESULT_SUCC;
}

eResultCode Network::InnerWrite(CConnector *pConnector, char *pSendData, int nSendDataSize)
{
	if( !pConnector )
	{
		WriteMiniNetLog(FormatW(L"error: MiniNet::InnerWrite(): Invalid session object %08X %d", pConnector, (pConnector ? pConnector->GetIndex() : -1)));
		eResultCode::RESULT_FAIL;
	}
	if( INVALID_SOCKET == pConnector->GetSocket() )
	{
		WriteMiniNetLog(FormatW(L"error: MiniNet::InnerWrite(): <%d> Invalid socket %d", pConnector->GetIndex(), pConnector->GetSocket()));
		eResultCode::RESULT_FAIL;
	}

	// 데이터 셋팅
	pConnector->AddInnerQueue(pSendData, nSendDataSize);

	//
	if( 0 >= pConnector->GetInnerRef() )
	{
		// 지금처리
		if( 0 > pConnector->InnerPrepare() )
		{
			WriteMiniNetLog(FormatW(L"error: MiniNet::InnerWrite(): <%d> InnerPrepare() fail", pConnector->GetIndex()));
			return eResultCode::RESULT_FAIL;
		}

		BOOL bRet = PostQueuedCompletionStatus(_hIOCP, nSendDataSize, (ULONG_PTR)pConnector, pConnector->GetInnerOverlapped());
		if( 0 == bRet )
		{
			int nErrorCode = GetLastError();
			WriteMiniNetLog(FormatW(L"error: MiniNet::InnerWrite(): <%d> PostQueuedCompletionStatus() fail. socket:%d. error:%d", pConnector->GetIndex(), pConnector->GetSocket(), nErrorCode));
			return eResultCode::RESULT_FAIL;
		}
	}

	//
	return eResultCode::RESULT_SUCC;
}

//
void WritePacketLog(std::wstring str, const char *pPacketData, int nPacketDataSize)
{
	return;

	//
	WCHAR wcsLogBuffer[MAX_PACKET_BUFFER_SIZE + 1024 + 1] = {0,};

	int nLen = 0;
	nLen += _snwprintf_s(wcsLogBuffer+nLen, MAX_PACKET_BUFFER_SIZE, MAX_PACKET_BUFFER_SIZE -nLen, L"%s", str.c_str());

	if( 1024 < nPacketDataSize )
	{
		for( int idx = 0; idx < 6; ++idx )
			nLen += _snwprintf_s(wcsLogBuffer + nLen, MAX_PACKET_BUFFER_SIZE, MAX_PACKET_BUFFER_SIZE - nLen, L"%02X", pPacketData[idx]);
		nLen += _snwprintf_s(wcsLogBuffer + nLen, MAX_PACKET_BUFFER_SIZE, MAX_PACKET_BUFFER_SIZE - nLen, L". connot write packetlog. too long.");
	}
	else
	{
		for( int idx = 0; idx < nPacketDataSize; ++idx )
			nLen += _snwprintf_s(wcsLogBuffer + nLen, MAX_PACKET_BUFFER_SIZE, MAX_PACKET_BUFFER_SIZE - nLen, L"%02X", pPacketData[idx]);
		nLen += _snwprintf_s(wcsLogBuffer + nLen, MAX_PACKET_BUFFER_SIZE, MAX_PACKET_BUFFER_SIZE - nLen, L"\0");
	}

	WriteMiniNetLog(wcsLogBuffer);
}