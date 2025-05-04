#pragma once
#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "./_common.h"

#include <queue>
#include <string>
#include <set>
#include <list>
#include <array>

#include <thread>
#include <atomic>
#include <functional>
#include <source_location>

using namespace std;

//
class Connector;
class NetworkBuffer;
struct OverlappedEx;

struct HostInfo
{
    WCHAR wcsIP[NetworkConst::MAX_LEN_IP4_STRING + 1]{0,};
    WORD wPort{0};
};

struct NetworkConfig
{
    int workerThreadCount{4};

    bool doListen{true};
    bool useAcceptEx{true};
    int nAcceptPrepareCount{10};

    HostInfo listenInfo;
};

//
class Network
{
public:
    NetworkConfig _config;

    HANDLE _hIOCP{INVALID_HANDLE_VALUE};

    SOCKET _ListenSock{INVALID_SOCKET};
    SOCKADDR_IN _ListenAddr{};

    //
    list<thread> _threads{};
    stop_source _threadStop;
    atomic<int> _threadSuspended{1};
    atomic<int> _threadWait{0};
    INT64 _biUpdateTime{0};

    //
private:
    Network(void);
    virtual ~Network(void);

public:
    static Network& GetInstance()
    {
        static Network* pInstance = new Network;
        return *pInstance;
    }

    bool Initialize();
    bool Finalize();

    bool Start();
    bool Stop();

    eResultCode WorkerThread(stop_token token);
    eResultCode AcceptThread(stop_token token);
    eResultCode UpdateThread(stop_token token);

    eResultCode DoUpdate(INT64 biCurrTime);

    bool lookup_host(const char* hostname, std::string& hostIP);
    Connector* Connect(WCHAR* pwcszIP, const WORD wPort);
    bool Disconnect(Connector* pConnector);
    void Disconnect(SOCKET socket);

    bool InitListenSocket();

    void AcceptPrepare(int nCount);
    void AcceptPrepare();

    eResultCode Write(Connector* pConnector, char* pSendData, int iSendDataSize);

    void DoAccept(Connector* pConnector, OverlappedEx* pOverlappedEx, DWORD dwSendCompleteSize);
    void DoRecv(Connector* pConnector, OverlappedEx* pOverlappedEx, DWORD dwRecvCompleteSize);
    void DoSend(Connector* pConnector, OverlappedEx* pOverlappedEx, DWORD dwSendCompleteSize);
};

//
#endif //__NETWORK_H__