#pragma once
#ifndef __CONNECTOR_H__
#define __CONNECTOR_H__

#include "./_common.h"
//#include <winsock2.h>
//#include <WS2tcpip.h>
//#include <Windows.h>

#include "../_lib/util_string.h"
#include "../_lib/safeLock.h"
#include "../_lib/log.h"

#include "./_common.h"
#include "./buffer.h"

#include <unordered_map>
#include <queue>
#include <list>
#include <atomic>

//
using namespace std;

class Connector;
struct OVERLAPPED_EX
{
    OVERLAPPED overlapped{};
    Connector* pConnector{nullptr};
    CNetworkBuffer* pBuffer{nullptr};

    //
    void ResetOverlapped() { memset(&overlapped, 0, sizeof(OVERLAPPED)); }
};

class Connector
{
private:
    Lock _lock;

    DWORD _dwUid{0};
    void* _param{nullptr};

    SOCKET _socket{INVALID_SOCKET};

    char _szDomain[eNetwork::MAX_LEN_DOMAIN_STRING + 1]{0,};
    WCHAR _wcsDomain[eNetwork::MAX_LEN_DOMAIN_STRING + 1]{0,};
    WORD _wPort{0};

    // send
    OVERLAPPED_EX _SendRequest;
    Lock _SendQueueLock;
    queue<CNetworkBuffer*> _SendQueue{}; // 여기있는걸 send 합니다. 최대 갯수를 넘어가면 전송량에 비해 보내야되는 패킷이 많은것
    DWORD _dwSendRef{0};

    // recv
    OVERLAPPED_EX _RecvRequest;
    CCircleBuffer _RecvDataBuffer; // recv 받으면 여기에 쌓습니다. 오버되면 연결을 터트립니다.
    DWORD _dwRecvRef{0};
    int (*funcDataParser)(CCircleBuffer& buffer) { nullptr };

    //// inner
    //OVERLAPPED_EX _InnerRequest;
    //Lock _InnerQueueLock;
    //queue<CNetworkBuffer*> _InnerQueue{}; //
    //DWORD _dwInnerRef{0};

    //
    INT64 _biUpdateTimer{0};
    INT64 _biHeartbeatTimer{0};

public:
    SOCKADDR_IN _SockAddr{};

protected:
    //DWORD _dwIndex{0};

    //
public:
    Connector()
    {
        static atomic<DWORD> unique_connector_uid{0};
        _dwUid = ++unique_connector_uid;
    }
    Connector(const Connector&) = delete;
    Connector& operator=(Connector&) = delete;
    virtual ~Connector()
    {
        Release();
    }

    void Release();
    void TryRelease();

    const DWORD GetUID() { return _dwUid; }
    const SOCKET SetSocket(SOCKET socket) { return _socket = socket; }
    const SOCKET GetSocket() { return _socket; }
    const bool GetActive() { return INVALID_SOCKET != _socket; }

    void SetDomainA(char* pszDomain, WORD wPort);
    void SetDomain(WCHAR* pwcsDomain, WORD wPort);
    const char* GetDomainA() { return _szDomain; }
    const WCHAR* GetDomain() { return _wcsDomain; };
    const WORD GetPort() { return _wPort; }

    void GetSocket2IP(char* pszIP);
    void ConvertSocket2IP();

    //void IncRef() { InterlockedIncrement((DWORD*)&_refCnt); }
    //void DecRef() { InterlockedDecrement((DWORD*)&_refCnt); }

    void* SetParam(void* param) { return _param = param; }
    void* GetParam() { return _param; }
    bool GetUsed() { return (nullptr != _param); }

    // send
    eResultCode AddSendData(char* pSendData, DWORD dwSendDataSize);
    int AddSendQueue(char* pSendData, DWORD dwSendDataSize); // ret: sendqueue.size
    int SendPrepare(); //ret: send data size
    int SendComplete(DWORD dwSendSize); // ret: remain send data size

    WSABUF* GetSendWSABuffer();
    OVERLAPPED* GetSendOverlapped() { return (OVERLAPPED*)&_SendRequest; }
    DWORD IncSendRef() { InterlockedIncrement(&_dwSendRef); return _dwSendRef; }
    DWORD DecSendRef() { InterlockedDecrement(&_dwSendRef); return _dwSendRef; }
    DWORD GetSendRef() { return _dwSendRef; }

    // recv
    int RecvPrepare(); //ret: recv buffer size
    int RecvComplete(DWORD dwRecvSize); // ret: recvdatabuffer.getdatasize
    void SetDataParser(int (*funcDataParserPtr)(CCircleBuffer& buffer)) { funcDataParser = funcDataParserPtr; }
    int DataParser();
    int DataParsing();

    WSABUF* GetRecvWSABuffer();
    OVERLAPPED* GetRecvOverlapped() { return (OVERLAPPED*)&_RecvRequest; }
    DWORD IncRecvRef() { InterlockedIncrement(&_dwRecvRef); return _dwRecvRef; }
    DWORD DecRecvRef() { InterlockedDecrement(&_dwRecvRef); return _dwRecvRef; }
    DWORD GetRecvRef() { return _dwRecvRef; }

    //// inner
    //int AddInnerQueue(char* pSendData, DWORD dwSendDataSize);
    //int InnerPrepare();
    //int InnerComplete(DWORD dwInnerSize);

    //WSABUF* GetInnerWSABuffer();
    //OVERLAPPED* GetInnerOverlapped() { return (OVERLAPPED*)&_InnerRequest; }

    //DWORD IncInnerRef() { InterlockedIncrement(&_dwInnerRef); return _dwInnerRef; }
    //DWORD DecInnerRef() { InterlockedDecrement(&_dwInnerRef); return _dwInnerRef; }
    //DWORD GetInnerRef() { return _dwInnerRef; }

    //
    bool CheckUpdateTimer(INT64 biCurrTime) { return (_biUpdateTimer < biCurrTime); }
    bool DoUpdate(INT64 biCurrTime);
    bool CheckHeartbeat(INT64 biCurrTime);
    wstring GetReport();
};

////
//class ConnectorMgr
//{
//private:
//    DWORD _dwConnectorUID{0};
//    DWORD _dwConnectorIndex{0};
//    DWORD _dwPoolSize{0};
//
//    Lock _lock;
//    list<Connector*> _poolList{}; // 생성된 전체 리스트
//    list<Connector*> _freeList{}; // 사용할 수 있는
//    //list<Connector*> _usedList{}; // 사용중인
//    unordered_map<DWORD, Connector*> _usedList{};
//    list<Connector*> _releaseList{}; // 정리예정 리스트
//
//public:
//
//    //
//private:
//    ConnectorMgr(DWORD poolSize = 5)
//    {
//        AllocPool(poolSize);
//    }
//
//    ~ConnectorMgr(void)
//    {
//        _usedList.clear();
//        _freeList.clear();
//        //_poolList.clear();
//        for (Connector* p : _poolList) {
//            delete p;
//        }
//    }
//
//    DWORD AllocPool(DWORD allocSize)
//    {
//        SafeLock lock(_lock);
//
//        for (DWORD cnt = 0; cnt < allocSize; ++cnt) {
//            //Connector* p = new Connector(MakeUID());
//            Connector* p = new Connector();
//            _poolList.emplace_back(p);
//            _freeList.emplace_back(p);
//        }
//        return _dwPoolSize = (DWORD)_poolList.size();
//    }
//
//public:
//    static ConnectorMgr& GetInstance()
//    {
//        static ConnectorMgr* pInstance = new ConnectorMgr();
//        return *pInstance;
//    }
//
//    //DWORD MakeUID()
//    //{
//    //    InterlockedIncrement((DWORD*)&_dwConnectorUID);
//    //    return _dwConnectorUID;
//    //}
//
//    DWORD MakeIndex()
//    {
//        InterlockedIncrement((DWORD*)&_dwConnectorIndex);
//        return _dwConnectorIndex;
//    }
//
//    Connector* GetFreeConnector()
//    {
//        if (_freeList.empty() && !_releaseList.empty()) {
//            DoUpdate();
//        }
//        if (_freeList.empty()) {
//            AllocPool(_dwPoolSize);
//            Log(format("ConnectorMgr pool doubling {}", _dwPoolSize));
//        }
//
//        SafeLock lock(_lock);
//        Connector* pConnector = _freeList.front();
//        _freeList.pop_front();
//
//        //pConnector->SetIndex(MakeIndex());
//        //_usedList.insert({pConnector->GetIndex(), pConnector});
//
//        return pConnector;
//    }
//
//    void ReleaseConnector(Connector* pConnector)
//    {
//        SafeLock lock(_lock);
//
//        _releaseList.emplace_back(pConnector);
//        if (_releaseList.size() > 10) {
//            DoUpdate();
//        }
//    }
//
//    void DoUpdate()
//    {
//        list<Connector*> releaseList{};
//        {
//            SafeLock lock(_lock);
//            releaseList.swap(_releaseList);
//        }
//
//        {
//            SafeLock lock(_lock);
//            for (auto v : releaseList) {
//                v->Release();
//
//                if (auto it = _usedList.find(v->GetUID()); it != _usedList.end()) {
//                    _usedList.erase(it);
//                    _freeList.emplace_back(v);
//                } else {
//                    LogError(format("invalid connector object. uid: {}", v->GetUID()));
//                }
//            }
//        }
//    }
//
//    wstring GetStateReport()
//    {
//        SafeLock lock(_lock);
//        //wstring wstrState{ FormatW(L"pool:%d, used:%d, free:%d", _poolList.size(), _usedList.size(), _freeList.size()) };
//        //return wstrState;
//        return format(L"pool:{}, used:{}, free:{}", _poolList.size(), _usedList.size(), _freeList.size());
//    }
//};

int DefaultDataParser(CCircleBuffer& buffer);

//
#endif //__CONNECTOR_H__