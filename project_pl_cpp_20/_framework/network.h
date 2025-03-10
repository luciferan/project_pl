﻿#pragma once
#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "./_common.h"
//#pragma comment(lib, "ws2_32.lib")

//#include <winsock2.h>
//#include <ws2tcpip.h>
//#include <windows.h>
//#include <process.h>
//#include <wchar.h>

#include <queue>
#include <string>
#include <set>
#include <list>
#include <array>

#include <thread>
#include <atomic>
#include <functional>
#include <source_location>

////
//extern void WriteMiniNetLog(std::wstring wstr);
//extern void WritePacketLog(std::wstring str, const char* pPacketData, int nPacketDataSize);
//extern void Log(const std::wstring& wstr);
//extern void Log(const std::string& str);
//extern void ErrorLog(const std::string& str, std::source_location loc = std::source_location::current());

//
using namespace std;

class Connector;
class CNetworkBuffer;

struct HostInfo
{
    WCHAR wcsIP[eNetwork::MAX_LEN_IP4_STRING + 1]{0,};
    WORD wPort{0};
};

struct NetworkConfig
{
    int workerThreadCount{4};
    bool doListen{true};

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

    unsigned int WorkerThread(stop_token token);
    unsigned int AcceptThread(stop_token token);
    unsigned int UpdateThread(stop_token token);

    eResultCode DoUpdate(INT64 biCurrTime);

    bool lookup_host(const char* hostname, std::string& hostIP);
    Connector* Connect(WCHAR* pwcszIP, const WORD wPort);
    bool Disconnect(Connector* pConnector);
    void Disconnect(SOCKET socket);

    eResultCode Write(Connector* pConnector, char* pSendData, int iSendDataSize);
    //eResultCode InnerWrite(Connector* pConnector, char* pSendData, int nSendDataSize);

    void DoSend(Connector* pConnector, CNetworkBuffer* pNetworkBuffer, DWORD dwSendCompleteSize);
    void DoRecv(Connector* pConnector, CNetworkBuffer* pNetworkBuffer, DWORD dwRecvCompleteSize);
};

//
#endif //__NETWORK_H__