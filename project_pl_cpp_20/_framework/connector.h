#pragma once
#ifndef __CONNECTOR_H__
#define __CONNECTOR_H__

//
#include <winsock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

#include <unordered_map>
#include <queue>
#include <list>
#include <atomic>

#include "../_lib/log.h"

#include "../_lib/safeLock.h"
#include "../_lib/util_string.h"

#include "./_common_variable.h"

#include "./buffer.h"

//
using namespace std;
class CConnector;

//
struct OVERLAPPED_EX
{
    OVERLAPPED overlapped{0,};

    CConnector* pSession{nullptr};
    CNetworkBuffer* pBuffer{nullptr};

    //
    void ResetOverlapped() { memset(&overlapped, 0, sizeof(OVERLAPPED)); }
};

//
class CConnector
{
protected:
    DWORD _dwUid{0};
    DWORD _dwIndex{0};

    void* _param{nullptr};

    SOCKET _socket{INVALID_SOCKET};

    int _refCnt{0};

public:
    SOCKADDR_IN _SockAddr{0,};

    char _szDomain[eNetwork::MAX_LEN_DOMAIN_STRING + 1]{0,};
    WCHAR _wcsDomain[eNetwork::MAX_LEN_DOMAIN_STRING + 1]{0,};
    WORD _wPort{0};

    // send
    OVERLAPPED_EX _SendRequest;
    Lock _SendQueueLock;
    queue<CNetworkBuffer*> _SendQueue{}; // �����ִ°� send �մϴ�. �ִ� ������ �Ѿ�� ���۷��� ���� �����ߵǴ� ��Ŷ�� ������
    DWORD _dwSendRef{0};

    // recv
    OVERLAPPED_EX _RecvRequest;
    CCircleBuffer _RecvDataBuffer; // recv ������ ���⿡ �׽��ϴ�. �����Ǹ� ������ ��Ʈ���ϴ�.
    DWORD _dwRecvRef{0};

    // inner
    OVERLAPPED_EX _InnerRequest;
    Lock _InnerQueueLock;
    queue<CNetworkBuffer*> _InnerQueue{}; //
    DWORD _dwInnerRef{0};

    //
    INT64 _biUpdateTimer{0};
    INT64 _biHeartbeatTimer{0};

    //
public:
    CConnector() = delete;
    //CConnector(const CConnector&) = delete;
    CConnector(DWORD uid = 0);
    virtual ~CConnector();

    bool Initialize();
    bool Release();

    DWORD GetUID() const { return _dwUid; }
    DWORD SetIndex(DWORD dwIndex) { return _dwIndex = dwIndex; }
    DWORD GetIndex() const { return _dwIndex; }

    void IncRef() { InterlockedIncrement((DWORD*)&_refCnt); }
    void DecRef() { InterlockedDecrement((DWORD*)&_refCnt); }

    void* SetParam(void* param) { return _param = param; }
    void* GetParam() { return _param; }
    bool GetUsed() { return (nullptr != _param); }

    SOCKET SetSocket(SOCKET socket) { return _socket = socket; }
    SOCKET GetSocket() { return _socket; }
    bool GetActive() { return (INVALID_SOCKET != _socket); }

    void SetDomain(WCHAR* pwcsDomain, WORD wPort);
    void GetSocket2IP(char* pszIP);
    void ConvertSocket2IP();

    char* GetDomainA() { return _szDomain; }
    WCHAR* GetDomain() { return _wcsDomain; }
    WORD GetPort() { return _wPort; }

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
    virtual int DataParsing();

    WSABUF* GetRecvWSABuffer();
    OVERLAPPED* GetRecvOverlapped() { return (OVERLAPPED*)&_RecvRequest; }
    DWORD IncRecvRef() { InterlockedIncrement(&_dwRecvRef); return _dwRecvRef; }
    DWORD DecRecvRef() { InterlockedDecrement(&_dwRecvRef); return _dwRecvRef; }
    DWORD GetRecvRef() { return _dwRecvRef; }

    // inner
    int AddInnerQueue(char* pSendData, DWORD dwSendDataSize);
    int InnerPrepare();
    int InnerComplete(DWORD dwInnerSize);

    WSABUF* GetInnerWSABuffer();
    OVERLAPPED* GetInnerOverlapped() { return (OVERLAPPED*)&_InnerRequest; }
    DWORD IncInnerRef() { InterlockedIncrement(&_dwInnerRef); return _dwInnerRef; }
    DWORD DecInnerRef() { InterlockedDecrement(&_dwInnerRef); return _dwInnerRef; }
    DWORD GetInnerRef() { return _dwInnerRef; }

    //
    bool CheckUpdateTimer(INT64 biCurrTime) { return (_biUpdateTimer < biCurrTime); }
    bool DoUpdate(INT64 biCurrTime);
    bool CheckHeartbeat(INT64 biCurrTime);
    wstring GetStateReport();
};

//
class CConnectorMgr
{
private:
    DWORD _dwConnectorUID{0};
    DWORD _dwConnectorIndex{0};
    DWORD _dwPoolSize{0};

    Lock _lock;
    list<CConnector*> _poolList{}; // ������ ��ü ����Ʈ
    list<CConnector*> _freeList{}; // ����� �� �ִ�
    //list<CConnector*> _usedList{}; // �������
    unordered_map<DWORD, CConnector*> _usedList{};
    list<CConnector*> _releaseList{}; // �������� ����Ʈ

public:

    //
private:
    CConnectorMgr(DWORD poolSize = 5)
    {
        AllocPool(poolSize);
    }

    ~CConnectorMgr(void)
    {
        _usedList.clear();
        _freeList.clear();
        //_poolList.clear();
        for (CConnector* p : _poolList) {
            delete p;
        }
    }

    DWORD AllocPool(DWORD allocSize)
    {
        SafeLock lock(_lock);

        for (DWORD cnt = 0; cnt < allocSize; ++cnt) {
            CConnector* p = new CConnector(MakeUID());
            _poolList.emplace_back(p);
            _freeList.emplace_back(p);
        }
        return _dwPoolSize = (DWORD)_poolList.size();
    }

public:
    static CConnectorMgr& GetInstance()
    {
        static CConnectorMgr* pInstance = new CConnectorMgr();
        return *pInstance;
    }

    DWORD MakeUID()
    {
        InterlockedIncrement((DWORD*)&_dwConnectorUID);
        return _dwConnectorUID;
    }

    DWORD MakeIndex()
    {
        InterlockedIncrement((DWORD*)&_dwConnectorIndex);
        return _dwConnectorIndex;
    }

    CConnector* GetFreeConnector()
    {
        if (_freeList.empty() && !_releaseList.empty()) {
            DoUpdate();
        }
        if (_freeList.empty()) {
            AllocPool(_dwPoolSize);
            Log(format("ConnectorMgr pool doubling {}", _dwPoolSize));
        }

        SafeLock lock(_lock);
        CConnector* pConnector = _freeList.front();
        _freeList.pop_front();

        pConnector->SetIndex(MakeIndex());
        _usedList.insert({pConnector->GetIndex(), pConnector});

        return pConnector;
    }

    void ReleaseConnector(CConnector* pConnector)
    {
        SafeLock lock(_lock);

        _releaseList.emplace_back(pConnector);
        if (_releaseList.size() > 10) {
            DoUpdate();
        }
    }

    void DoUpdate()
    {
        list<CConnector*> releaseList{};
        {
            SafeLock lock(_lock);
            releaseList.swap(_releaseList);
        }

        {
            SafeLock lock(_lock);
            for (auto v : releaseList) {
                v->Release();

                if (auto it = _usedList.find(v->GetIndex()); it != _usedList.end()) {
                    _usedList.erase(it);
                    _freeList.emplace_back(v);
                } else {
                    LogError(format("invalid connector object. uid: {}, index: {}", v->GetUID(), v->GetIndex()));
                }
            }
        }
    }

    wstring GetStateReport()
    {
        SafeLock lock(_lock);
        //wstring wstrState{ FormatW(L"pool:%d, used:%d, free:%d", _poolList.size(), _usedList.size(), _freeList.size()) };
        //return wstrState;
        return format(L"pool:{}, used:{}, free:{}", _poolList.size(), _usedList.size(), _freeList.size());
    }
};

//
#endif //__CONNECTOR_H__