#pragma once
#ifndef __USER_SESSION_H__
#define __USER_SESSION_H__

#include <Windows.h>

#include "../_framework/_common.h"
#include "../_framework/character_base.h"
#include "../_lib/safe_lock.h"

#include "./packet_svr.h"
#include "./zone.h"

#include <list>
#include <queue>

using namespace std;

//
class Connector;
class UserSession
{
private:
    Lock _lock;

    DWORD _dwUid{0};
    Connector* _pConnector{nullptr};

    INT64 _biHeartbeatTimer{0};
    unsigned int _nHeartbeatCount{0};
    INT64 _biUpdateTime{0};

public:
    Character _nub;

public:
    bool _bAuthed{false};

    //
public:
    UserSession();
    UserSession(const UserSession&) = delete;
    UserSession& operator=(UserSession&) = delete;
    virtual ~UserSession();

    void Release();

    //
    DWORD GetUID() { return _dwUid; }
    bool GetActive() { return _bAuthed; }
    bool GetUsed() { return (nullptr != _pConnector); }

    void SetConnector(Connector* pConnector) { _pConnector = pConnector; }
    Connector* GetConnector() { return _pConnector; }

    void SetUpdateTime(INT64 biCurrTime) { _biUpdateTime = biCurrTime; }
    INT64 GetUpdateTime() { return _biUpdateTime; }
    bool CheckUpdateTime(INT64 biCurrTime) { return (_biUpdateTime > biCurrTime); }

    Lock& GetLock() { return _lock; }

    INT64 GetHeartbeatTimer() { return _biHeartbeatTimer; }
    void SetHeartbeatTimer(INT64 biCurrTime) { _biHeartbeatTimer = biCurrTime; }
    int GetHeartbeat() { return _nHeartbeatCount;  }
    int IncHeartbeat() { return InterlockedIncrement(&_nHeartbeatCount); }
    int DecHeartbeat() { return InterlockedDecrement(&_nHeartbeatCount); }

    const INT64 GetToken() { return _nub.GetToken(); }
    Character* GetCharacter() { return &_nub; }

    //
    eResultCode SendPacket(PacketBaseS2C* packetData, DWORD packetSize);
    eResultCode SendPacket(char* packetBuffer, DWORD packetSize);

    //
    void DoUpdate(SafeLock&, INT64 uiCurrTime);
    bool MessageProcess(char* pData, int nLen);

public:
    typedef PacketHandler<UserSession, static_cast<int>(PacketTypeC2S::Max)> HandlerImpl;
    static HandlerImpl _impl;
};

//
#endif //__USER_SESSION_H__