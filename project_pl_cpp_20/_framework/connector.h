#pragma once
#ifndef __CONNECTOR_H__
#define __CONNECTOR_H__

#include "./_common.h"

#include "../_lib/util_string.h"
#include "../_lib/safe_lock.h"
#include "../_lib/log.h"

#include "./buffer.h"

#include <unordered_map>
#include <queue>
#include <list>
#include <atomic>

using namespace std;

//
enum class eNetworkOperator : unsigned char
{
    OP_SEND = 1,
    OP_RECV = 2,
    OP_ACCEPT = 3,
};

class Connector;
struct OverlappedEx
{
    OVERLAPPED overlapped{};
    Connector* pConnector{nullptr};
    eNetworkOperator NetOp{eNetworkOperator::OP_SEND};

    void ResetOverlapped() { memset(&overlapped, 0, sizeof(OVERLAPPED)); }
    eNetworkOperator GetOperator() { return NetOp; }
};

struct SendOverlapped : public OverlappedEx
{
    NetworkBuffer* pBuffer{nullptr};
};

struct RecvOverlapped : public OverlappedEx
{
    unique_ptr<NetworkBuffer> pBuffer = make_unique<NetworkBuffer>();
};

class Connector
{
private:
    Lock _lock;

    DWORD _dwUid{0};
    void* _param{nullptr};

    SOCKET _socket{INVALID_SOCKET};

    char _szDomain[NetworkConst::MAX_LEN_DOMAIN_STRING + 1]{0,};
    WCHAR _wcsDomain[NetworkConst::MAX_LEN_DOMAIN_STRING + 1]{0,};
    WORD _wPort{0};

    // send
    SendOverlapped _SendRequest;
    Lock _SendQueueLock;
    queue<NetworkBuffer*> _SendQueue{}; // 여기있는걸 send 합니다. 최대 갯수를 넘어가면 전송량에 비해 보내야되는 패킷이 많은것
    DWORD _dwSendRef{0};

    // recv
    RecvOverlapped _RecvRequest;
    CircleBuffer _RecvDataBuffer; // recv 받으면 여기에 쌓습니다. 오버되면 연결을 터트립니다.
    DWORD _dwRecvRef{0};
    int (*funcDataParser)(CircleBuffer& buffer) { nullptr };

    //
    INT64 _biUpdateTimer{0};
    INT64 _biHeartbeatTimer{0};

public:
    SOCKADDR_IN _SockAddr{};

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
    void SetDataParser(int (*funcDataParserPtr)(CircleBuffer& buffer)) { funcDataParser = funcDataParserPtr; }
    int DataParser();
    int DataParsing();

    WSABUF* GetRecvWSABuffer();
    OVERLAPPED* GetRecvOverlapped() { return (OVERLAPPED*)&_RecvRequest; }
    DWORD IncRecvRef() { InterlockedIncrement(&_dwRecvRef); return _dwRecvRef; }
    DWORD DecRecvRef() { InterlockedDecrement(&_dwRecvRef); return _dwRecvRef; }
    DWORD GetRecvRef() { return _dwRecvRef; }

    // accept
    int AcceptPrepare();

    //
    bool CheckUpdateTimer(INT64 biCurrTime) { return (_biUpdateTimer < biCurrTime); }
    bool DoUpdate(INT64 biCurrTime);
    bool CheckHeartbeat(INT64 biCurrTime);

    string GetReportA();
    wstring GetReport();
};

int DefaultDataParser(CircleBuffer& buffer);

//
#endif //__CONNECTOR_H__