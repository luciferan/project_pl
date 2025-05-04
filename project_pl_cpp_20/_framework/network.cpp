#include "stdafx.h"

#include "./network.h"
#include "./connector_mgr.h"
#include "./buffer.h"
#include "./packet_data_queue.h"

#include "../_lib/exception_report.h"
#include "../_lib/util.h"
#include "../_lib/log.h"

#include <iostream>
#include <string>
#include <format>
#include <source_location>

//
LPFN_ACCEPTEX acceptEx{nullptr};

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
    bool bRet{false};

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
    // worketThread
    for (int idx = 0; idx < _config.workerThreadCount; ++idx) {
        _threads.emplace_back(thread{&Network::WorkerThread, this, _threadStop.get_token()});
    }

    // acceptThread
    if (_config.doListen) {
        if (_config.useAcceptEx) {
            InitListenSocket();
            AcceptPrepare(_config.nAcceptPrepareCount);
        } else {
            _threads.emplace_back(thread{&Network::AcceptThread, this, _threadStop.get_token()});
        }
    }

    // UpdateThread
    {
        _threads.emplace_back(thread{&Network::UpdateThread, this, _threadStop.get_token()});
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

eResultCode Network::AcceptThread(stop_token token)
{
    Log(format("log: {} create", "Network::AcceptThread"));
    _threadSuspended.wait(1);

    //
    ConnectorMgr& ConnectMgr{ConnectorMgr::GetInstance()};
    Connector* pConnector{nullptr};

    HANDLE& hNetworkHandle{_hIOCP};
    NetworkConfig& config{_config};

    INT32 iWSARet{0};
    HANDLE hRet{INVALID_HANDLE_VALUE};
    BOOL bRet{FALSE};

    DWORD dwRecvDataSize{0};
    DWORD dwFlag{0};

    //
    SOCKET listenSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (INVALID_SOCKET == listenSock) {
        Log("error: WSASocket fail.");
        return eResultCode::fail;
    }

    SOCKADDR_IN listenAddr = {0,};
    memset(&listenAddr, 0, sizeof(listenAddr));
    listenAddr.sin_family = AF_INET;
    listenAddr.sin_addr.s_addr = /*inet_addr(pszListenIP);*/ htonl(INADDR_ANY);
    listenAddr.sin_port = htons(_config.listenInfo.wPort);

    //
    if (SOCKET_ERROR == ::bind(listenSock, (SOCKADDR*)&listenAddr, sizeof(listenAddr))) {
        int nErrorCode = WSAGetLastError();
        LogError(format("bind fail. error {}", nErrorCode));
        return eResultCode::fail;
    }
    if (SOCKET_ERROR == listen(listenSock, 10)) {
        int nErrorCode = WSAGetLastError();
        LogError(format("listen fail. error {}", nErrorCode));
        return eResultCode::fail;
    }
    Log(format("log: accept bind port: {}", _config.listenInfo.wPort));

    _ListenSock = listenSock;
    _ListenAddr = listenAddr;

    //
    Log(format("log: {}: start", "Network::AcceptThread"));
    while (!token.stop_requested()) {
        SOCKADDR_IN AcceptAddr;
        memset(&AcceptAddr, 0, sizeof(AcceptAddr));
        int iAcceptAddrLen = sizeof(AcceptAddr);

        //
        SOCKET AcceptSock = WSAAccept(listenSock, (SOCKADDR*)&AcceptAddr, &iAcceptAddrLen, NULL, NULL);
        if (INVALID_SOCKET == AcceptSock) {
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
        pConnector = ConnectMgr.GetFreeObject(); // new Connector;
        if (!pConnector) {
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
        if (hNetworkHandle != hRet) {
            int nErrorCode = GetLastError();
            LogError(format("<{}> CreateIoCompletionPort() fail. socket:{}. error:{}", pConnector->GetUID(), AcceptSock, nErrorCode));
            closesocket(AcceptSock);
            ConnectMgr.SetFreeObject(pConnector); //SAFE_DELETE(pConnector);
            continue;
        }

        //pConnector->SetActive();

        //
        if (0 >= pConnector->RecvPrepare()) {
            LogError(format("<{}> RecvPrepare() fail. socket:{}", pConnector->GetUID(), AcceptSock));
            closesocket(AcceptSock);
            ConnectMgr.SetFreeObject(pConnector); //SAFE_DELETE(pSession);
            continue;
        }

        //
        dwFlag = 0;
        iWSARet = WSARecv(pConnector->GetSocket(), pConnector->GetRecvWSABuffer(), 1, &dwRecvDataSize, &dwFlag, pConnector->GetRecvOverlapped(), NULL);
        if (SOCKET_ERROR == iWSARet) {
            int nErrorCode = WSAGetLastError();
            if (WSA_IO_PENDING != nErrorCode) {
                LogError(format("<{}> WSARecv fail. socket:{}. error:{}", pConnector->GetUID(), AcceptSock, nErrorCode));
                closesocket(AcceptSock);
                ConnectMgr.SetFreeObject(pConnector); //SAFE_DELETE(pSession);
            }
        }
    }

    Log(format("log: {}: end", "Network::AcceptThread"));
    return eResultCode::succ;
}

eResultCode Network::WorkerThread(stop_token token)
{
    Log(format("log: {}: create", "Network::WorkerThread"));
    _threadSuspended.wait(1);

    ConnectorMgr& ConnectMgr{ConnectorMgr::GetInstance()};
    Connector* pConnector{nullptr};

    RecvPacketQueue& RecvPacketQueue{RecvPacketQueue::GetInstance()};

    //
    OVERLAPPED* lpOverlapped{nullptr};
    OverlappedEx* lpOverlappedEx{nullptr};
    int nSessionIndex{0};

    DWORD dwIOSize{0};
    DWORD dwFlags{0};

    BOOL bRet{FALSE};
    int iRet{0};
    int nErrorCode{0};

    NetworkBuffer* pNetworkBuffer{nullptr};

    DWORD dwSendDataSize{0};
    DWORD dwRecvDataSize{0};

    HANDLE& hNetworkHandle{_hIOCP};

    //
    Log(format("log: {}: start", "Network::WorkerThread"));
    while (!token.stop_requested()) {
        dwIOSize = 0;
        bRet = FALSE;

        bRet = GetQueuedCompletionStatus(hNetworkHandle, &dwIOSize, (PULONG_PTR)&pConnector, &lpOverlapped, 500);
        if (FALSE == bRet) {
            nErrorCode = WSAGetLastError();
            if (WAIT_TIMEOUT == nErrorCode) {
                continue;
            }
            Log(format("log: WorkerThread(): GetQueuedCompletionStatus() result false. lpOverlapped {:x}. error:{}", reinterpret_cast<intptr_t>(lpOverlapped), nErrorCode));

            //
            if (NULL == lpOverlapped) {
                continue;
            }

            if (pConnector) {
                Disconnect(pConnector);
            }

            continue;
        }

        //
        if (NULL == lpOverlapped) {
            Log("error: WorkerThread(): lpOverlapped is NULL");
            continue;
        }
        lpOverlappedEx = (OverlappedEx*)lpOverlapped;
        eNetworkOperator netOp = lpOverlappedEx->GetOperator();

        //
        pConnector = lpOverlappedEx->pConnector;
        if (!pConnector) {
            Log(format("error: WorkerThread(): Invalid connector {:x}", reinterpret_cast<intptr_t>(pConnector)));
            continue;
        }

        // disconnect
        if (netOp != eNetworkOperator::OP_ACCEPT && 0 == dwIOSize) {
            if (pConnector) {
                Log(FormatW(L"log: WorkerThread(): <%d> Disconnect. IP %s, Socket %d", pConnector->GetUID(), pConnector->GetDomain(), pConnector->GetSocket()));

                if (INVALID_SOCKET != pConnector->GetSocket()) {
                    shutdown(pConnector->GetSocket(), SD_BOTH);
                    closesocket(pConnector->GetSocket());
                }

                //
                if (pConnector->GetSendRef() || pConnector->GetRecvRef() ) {
                    switch (netOp) {
                    case eNetworkOperator::OP_SEND: pConnector->DecSendRef(); break;
                    case eNetworkOperator::OP_RECV: pConnector->DecRecvRef(); break;
                    }
                }

                if (0 >= pConnector->GetSendRef() || 0 >= pConnector->GetRecvRef() ) {
                    pConnector->TryRelease();
                }
            } else {
                Log(format("error: WorkerThread(): unknown connector. IoSize {}, pCurrSockEx {:x}", dwIOSize, reinterpret_cast<intptr_t>(lpOverlappedEx)));
            }

            continue;
        }

        //
        switch (netOp) {
        case eNetworkOperator::OP_ACCEPT:
            DoAccept(pConnector, lpOverlappedEx, dwIOSize);
            AcceptPrepare();
            break;
        case eNetworkOperator::OP_SEND:
            DoSend(pConnector, lpOverlappedEx, dwIOSize);
            break;
        case eNetworkOperator::OP_RECV:
            DoRecv(pConnector, lpOverlappedEx, dwIOSize);
            break;
        default:
            // 뭐여?
            LogError(format("WorkerThread(): <{}> Unknown operator: socket:{}. RecvDataSize {}", pConnector->GetUID(), pConnector->GetSocket(), dwIOSize));
            Disconnect(pConnector);
            break;
        }
    }

    Log(format("log: {}: end", "Network::WorkerThread"));
    return eResultCode::succ;
}

eResultCode Network::UpdateThread(stop_token token)
{
    Log(format("log: {}: create", "Network::UpdateThread"));
    _threadSuspended.wait(1);

    //
    ConnectorMgr& ConnectorMgr{ConnectorMgr::GetInstance()};
    Connector* pConnector{nullptr};

    //
    void* pParam{nullptr};

    INT64 biCurrTime{0};
    INT64 biReleaseCheck{GetTimeMilliSec() + (MILLISEC_A_SEC * 30)};

    std::list<Connector*> ReleaseConnectorList{};

    //
    Log(format("log: {}: start", "Network::UpdateThread"));
    while (!token.stop_requested()) {
        biCurrTime = GetTimeMilliSec();

        DoUpdate(biCurrTime);

        //
        this_thread::sleep_for(1ms);
    }

    Log(format("log: {}: end", "Network::UpdateThread"));
    return eResultCode::succ;
}

bool Network::lookup_host(const char* hostname, std::string& hostIP)
{
    ADDRINFO hints{};
    ADDRINFO* result{nullptr};

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;

    char szDomain[NetworkConst::MAX_LEN_DOMAIN_STRING + 1]{0,};
    strncpy_s(szDomain, NetworkConst::MAX_LEN_DOMAIN_STRING, hostname, NetworkConst::MAX_LEN_DOMAIN_STRING);

    int ret = getaddrinfo(szDomain, NULL, &hints, &result);
    if (0 == ret) {
        char addrstr[100]{0,};
        void* ptr{nullptr};
        inet_ntop(result->ai_family, result->ai_addr->sa_data, addrstr, _countof(addrstr));
        switch (result->ai_family) {
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
    } else {
        return false;
    }
}

Connector* Network::Connect(WCHAR* pwcsDomain, const WORD wPort)
{
    Connector* pConn = ConnectorMgr::GetInstance().GetFreeObject(); //new Connector;
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
    if (iswalpha(pwcsDomain[0])) {
        //struct hostent *host = gethostbyname(pConn->GetDomainA());
        //if( NULL == host )
        //{
        //	goto ProcFail;
        //}
        //SockAddr.sin_addr.s_addr = *(unsigned long*)host->h_addr_list[0];

        string strHostIP{};
        bool bRet = lookup_host(pConn->GetDomainA(), strHostIP);
        if (bRet) {
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
    if (INVALID_SOCKET == pConn->GetSocket()) {
        LogError("WSASocket() fail");
        //ConnectorMgr::GetInstance().SetFreeObject(pConn);
        pConn->TryRelease();
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
        //ConnectorMgr::GetInstance().SetFreeObject(pConn);
        pConn->TryRelease();
        return nullptr;
    }
    memcpy((void*)&pConn->_SockAddr, (void*)&SockAddr, sizeof(SockAddr));

    //Log(format("log: Network::Connect(): <{}> Domain: {}, Port: {}, Socket: {}", pConn->GetUID(), pwcsDomain, wPort, sock));

    //
    HANDLE hRet = CreateIoCompletionPort((HANDLE)sock, _hIOCP, (ULONG_PTR)pConn, 0);
    if (_hIOCP != hRet) {
        //ConnectorMgr::GetInstance().SetFreeObject(pConn);
        pConn->TryRelease();
        return nullptr;
    }

    // 데이터 수신 대기
    DWORD dwRecvDataSize = 0;
    if (0 >= pConn->RecvPrepare()) {
        Log(format("<{}> RecvPrepare() fail", pConn->GetUID()));
        //ConnectorMgr::GetInstance().SetFreeObject(pConn);
        pConn->TryRelease();
        return nullptr;
    }

    //
    DWORD dwFlags = 0;
    nRet = WSARecv(pConn->GetSocket(), pConn->GetRecvWSABuffer(), 1, &dwRecvDataSize, &dwFlags, pConn->GetRecvOverlapped(), NULL);
    if (SOCKET_ERROR == nRet) {
        int nErrorCode = WSAGetLastError();
        if (WSA_IO_PENDING != nErrorCode) {
            Log(format("error: Network::Connect(): WSARecv() fail. {}, Socket {}. error:{}", pConn->GetUID(), pConn->GetSocket(), nErrorCode));
            //ConnectorMgr::GetInstance().SetFreeObject(pConn);
            pConn->TryRelease();
            return nullptr;
        }
    }

    //
    return pConn;
}

bool Network::Disconnect(Connector* pConnector)
{
    if (!pConnector) {
        LogError(format("Invalid session object {:x}", reinterpret_cast<intptr_t>(pConnector)));
        return false;
    }

    //pConnector->SetDeactive();

    //
    Log(format("info: <{}> Disconnect. socket {}", pConnector->GetUID(), pConnector->GetSocket()));

    //
    if (INVALID_SOCKET != pConnector->GetSocket()) {
        shutdown(pConnector->GetSocket(), SD_BOTH);
        closesocket(pConnector->GetSocket());
        pConnector->SetSocket(INVALID_SOCKET);
    }

    //
    if (pConnector->GetRecvRef()) {
        PostQueuedCompletionStatus(_hIOCP, 0, (ULONG_PTR)pConnector, pConnector->GetRecvOverlapped());
    }

    //
    return true;
}

void Network::Disconnect(SOCKET socket)
{
    if (INVALID_SOCKET != socket) {
        shutdown(socket, SD_BOTH);
        closesocket(socket);
    }
}

eResultCode Network::Write(Connector* pConnector, char* pSendData, int iSendDataSize)
{
    if (!pConnector) {
        LogError(format("Invalid session object {:x} {}", reinterpret_cast<intptr_t>(pConnector), (pConnector ? pConnector->GetUID() : -1)));
        return eResultCode::fail;
    }
    if (INVALID_SOCKET == pConnector->GetSocket()) {
        LogError(format("<{}> Invalid socket {}", pConnector->GetUID(), pConnector->GetSocket()));
        return eResultCode::socket_disconnected;
    }

    // 전송데이터 셋팅
    if (0 > pConnector->AddSendQueue(pSendData, iSendDataSize)) {
        LogError(format("<{}> Invalid send data {}", pConnector->GetUID(), pConnector->GetSocket()));
        return eResultCode::fail;
    }

    //
    if (0 >= pConnector->GetSendRef()) {
        // 지금 전송
        if (0 > pConnector->SendPrepare()) {
            LogError(format("<{}> SendPrepare() fail", pConnector->GetUID()));
            return eResultCode::fail;
        }

        //
        DWORD dwSendDataSize = 0;
        DWORD dwFlags = 0;
        int nRet = WSASend(pConnector->GetSocket(), pConnector->GetSendWSABuffer(), 1, &dwSendDataSize, dwFlags, pConnector->GetSendOverlapped(), NULL);
        if (SOCKET_ERROR == nRet) {
            int nErrorCode = WSAGetLastError();
            if (WSA_IO_PENDING != nErrorCode) {
                LogError(format("<{}> WSASend() fail. socket:{}. error:{}", pConnector->GetUID(), pConnector->GetSocket(), nErrorCode));
                return eResultCode::fail;
            }
        }
    }

    //
    return eResultCode::succ;
}

void Network::DoSend(Connector* pConnector, OverlappedEx* pOverlappedEx, DWORD dwSendCompleteSize)
{
    SendOverlapped* pSendOverlapped = static_cast<SendOverlapped*>(pOverlappedEx);

    Log(format("debug: WorkerThread(): <{}> SendResult : socket:{}. SendRequestSize {}, SendSize {}", pConnector->GetUID(), pConnector->GetSocket(), (int)pSendOverlapped->pBuffer->_nDataSize, dwSendCompleteSize));
    PacketLog(format(L"debug: WorkerThread(): <{}> SendData:", pConnector->GetUID()), pSendOverlapped->pBuffer->_pBuffer, dwSendCompleteSize);

    DWORD dwSendDataSize{0};
    int nRemainData{pConnector->SendComplete(dwSendCompleteSize)};

    //
    if (0 < nRemainData || 0 < pConnector->SendPrepare()) {
        // 계속 전송
        int iRet = WSASend(pConnector->GetSocket(), pConnector->GetSendWSABuffer(), 1, &dwSendDataSize, 0, pConnector->GetSendOverlapped(), NULL);
        if (SOCKET_ERROR == iRet) {
            int nErrorCode = WSAGetLastError();
            if (WSA_IO_PENDING != nErrorCode) {
                Log(format("error: WorkerThread(): <{}> WSASend() fail. socket:{}. error:{}", pConnector->GetUID(), pConnector->GetSocket(), nErrorCode));
                Disconnect(pConnector);
            }
        }
    }
}

void Network::DoRecv(Connector* pConnector, OverlappedEx* pOverlappedEx, DWORD dwRecvCompleteSize)
{
    RecvOverlapped* pRecvOverlapped = static_cast<RecvOverlapped*>(pOverlappedEx);

    Log(format("debug: WorkerThread(): <{}> RecvResult : socket:{}. RecvDataSize {}", pConnector->GetUID(), pConnector->GetSocket(), dwRecvCompleteSize));
    //PacketLog(format(L"debug: WorkerThread(): <{}> RecvData: ", pConnector->GetUID()), pNetworkBuffer->_pBuffer, dwRecvCompleteSize);
    PacketLog(format(L"debug: WorkerThread(): <{}> RecvData: ", pConnector->GetUID()), pRecvOverlapped->pBuffer->_pBuffer, dwRecvCompleteSize);

    DWORD dwRecvDataSize{0};
    DWORD dwFlags{0};

    // 수신 완료
    int nResult = pConnector->RecvComplete(dwRecvCompleteSize);
    if (0 > nResult) {
        Log(format("error: WorkerThread(): <{}> Connector::RecvComplete() fail. socket:{}", pConnector->GetUID(), pConnector->GetSocket()));
        Disconnect(pConnector);
    } else {
        if (0 < pConnector->RecvPrepare()) {
            // 계속 수신
            int iRet = WSARecv(pConnector->GetSocket(), pConnector->GetRecvWSABuffer(), 1, &dwRecvDataSize, &dwFlags, pConnector->GetRecvOverlapped(), NULL);
            if (SOCKET_ERROR == iRet) {
                int nErrorCode = WSAGetLastError();
                if (WSA_IO_PENDING != nErrorCode) {
                    Log(format("error: WorkerThread(): <{}> WSARecv() fail. socket:{}. error:{}", pConnector->GetUID(), pConnector->GetSocket(), nErrorCode));
                    Disconnect(pConnector);
                }
            }
        } else {
            Log(format("error: WorkerThread(): <{}> Connector::RecvPrepare() fail. socket:{}", pConnector->GetUID(), pConnector->GetSocket()));
        }
    }
}

bool Network::InitListenSocket()
{
    SOCKET listenSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (INVALID_SOCKET == listenSock) {
        Log("error: WSASocket fail.");
        return false;
    }

    HANDLE hRet = CreateIoCompletionPort((HANDLE)listenSock, _hIOCP, NULL, NULL);
    if (hRet != _hIOCP) {
        Log("error: CreateIoCompletionPort fail.");
        return false;
    }

    SOCKADDR_IN listenAddr = {0,};
    memset(&listenAddr, 0, sizeof(listenAddr));
    listenAddr.sin_family = AF_INET;
    listenAddr.sin_addr.s_addr = /*inet_addr(pszListenIP);*/ htonl(INADDR_ANY);
    listenAddr.sin_port = htons(_config.listenInfo.wPort);

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

    _ListenSock = listenSock;

    GUID GuidAcceptEx = WSAID_ACCEPTEX;
    DWORD dwBytes{0};

    int iRet = WSAIoctl(listenSock, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx), &acceptEx, sizeof(acceptEx), &dwBytes, NULL, NULL);
    if (SOCKET_ERROR == iRet) {
        LogError(format("WSAIoctl fail. error {}", WSAGetLastError()));
        return false;
    }

    return true;
}

void Network::AcceptPrepare(int nCount /*= 10*/)
{
    for (int cnt = 0; cnt < nCount; ++cnt) {
        AcceptPrepare();
    }
}

void Network::AcceptPrepare()
{
    Connector* pConnector = ConnectorMgr::GetInstance().GetFreeObject();

    //
    SOCKET acceptSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (INVALID_SOCKET == acceptSocket) {
        return;
    }
    pConnector->SetSocket(acceptSocket);
    pConnector->AcceptPrepare();

    DWORD dwBytes{0};

    BOOL bRet = acceptEx(_ListenSock, pConnector->GetSocket(), pConnector->GetRecvWSABuffer(), 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, pConnector->GetRecvOverlapped());
    if (!bRet) {
        if (WSAGetLastError() != WSA_IO_PENDING) {
            Disconnect(pConnector);
            return;
        }
    }

    return;
}

void Network::DoAccept(Connector* pConnector, OverlappedEx* pOverlappedEx, DWORD dwSendCompleteSize)
{
    RecvOverlapped* pRecvOverlapped = static_cast<RecvOverlapped*>(pOverlappedEx);

    ConnectorMgr& ConnectMgr{ConnectorMgr::GetInstance()};

    int iRet{0};
    iRet = setsockopt(pConnector->GetSocket(), SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&_ListenSock, sizeof(_ListenSock));

    pConnector->_SockAddr;
    pConnector->ConvertSocket2IP();

    HANDLE hRet = CreateIoCompletionPort((HANDLE)pConnector->GetSocket(), _hIOCP, (ULONG_PTR)pConnector, 0);
    if (hRet != _hIOCP) {
        LogError(format("<{}> CreateIoCompletionPort() fail. socket:{}. error:{}", pConnector->GetUID(), pConnector->GetSocket(), GetLastError()));
        closesocket(pConnector->GetSocket());
        ConnectMgr.SetFreeObject(pConnector); //SAFE_DELETE(pSession);
        return;
    }

    if (0 >= pConnector->RecvPrepare()) {
        LogError(format("<{}> RecvPrepare() fail. socket:{}", pConnector->GetUID(), pConnector->GetSocket()));
        closesocket(pConnector->GetSocket());
        ConnectMgr.SetFreeObject(pConnector); //SAFE_DELETE(pSession);
        return;
    }

    DWORD dwFlag{0};
    DWORD dwRecvDataSize{0};
    INT32 iWSARet = WSARecv(pConnector->GetSocket(), pConnector->GetRecvWSABuffer(), 1, &dwRecvDataSize, &dwFlag, pConnector->GetRecvOverlapped(), NULL);
    if (SOCKET_ERROR == iWSARet) {
        int nErrorCode = WSAGetLastError();
        if (WSA_IO_PENDING != nErrorCode) {
            LogError(format("<{}> WSARecv fail. socket:{}. error:{}", pConnector->GetUID(), pConnector->GetSocket(), nErrorCode));
            closesocket(pConnector->GetSocket());
            ConnectMgr.SetFreeObject(pConnector); //SAFE_DELETE(pSession);
        }
    }
}
