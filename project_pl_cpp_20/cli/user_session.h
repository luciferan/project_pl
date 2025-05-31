#pragma once
#ifndef __USER_SESSION_H__
#define __USER_SESSION_H__

#include <Windows.h>

#include "../_framework/_common.h"
#include "../_framework/character_base.h"
#include "../_lib/safe_lock.h"

#include "./packet_cli.h"

#include <list>
#include <queue>

using namespace std;

//
class Connector;
class UserSession
{
private:
    Lock _lock;

    DWORD _dwIndex{0};
    Connector* _pConnector{nullptr};

    INT64 _token{0};

    //
public:
    UserSession(DWORD dwIndex = 0)
        : _dwIndex(dwIndex)
    {
    }
    UserSession(const UserSession&) = delete;
    UserSession& operator=(UserSession&) = delete;
    virtual ~UserSession() {}

    int clear();

    DWORD GetIndex() { return _dwIndex; }
    void SetConnector(Connector* pConnector) { _pConnector = pConnector; }

    void SetToken(INT64 token) { _token = token; }
    INT64 GetToken() { return _token; }

    //
    eResultCode SendPacket(PacketBaseC2S* packetData, DWORD packetSize);

    //
    eResultCode ReqAuth();
    eResultCode ReqEnter();
    eResultCode ReqMove(int posX, int posY);
    eResultCode ReqInteraction(int targetId, int type);

    eResultCode RepHeartBeat();
    eResultCode ReqEcho();

    //
    int DoUpdate(INT64 uiCurrTime);
    int MessageProcess(char* pData, int nLen);

public:
    typedef PacketHandler<UserSession, static_cast<int>(PacketTypeS2C::Max)> HandlerImpl;
    static HandlerImpl _impl;
};

#endif //__USER_SESSION_H__