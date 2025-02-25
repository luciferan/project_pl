#pragma once
#ifndef __USER_SESSION_H__
#define __USER_SESSION_H__

#include <Windows.h>

#include "../_framework/_common.h"
#include "../_framework/packet_cli.h"
#include "../_lib/safeLock.h"

#include "../_framework/character.h"

#include <list>
#include <queue>

using namespace std;

//
class CConnector;
class CUserSession
{
private:
    DWORD _dwIndex{0};
    CConnector* _pConnector{nullptr};

    DWORD _dwUsed{0};
    DWORD _dwActive{0};

public:
    Character _nub;

    //
public:
    CUserSession(DWORD dwIndex = 0)
        : _dwIndex(dwIndex)
    {
    }
    virtual ~CUserSession() {}

    int clear();

    DWORD GetIndex() { return _dwIndex; }
    void SetConnector(CConnector* pConnector) { _pConnector = pConnector; }

    //
    int MessageProcess(char* pData, int nLen);

    int DoUpdate(INT64 uiCurrTime);
    eResultCode SendPacketData(DWORD dwProtocol, char* pData, DWORD dwDataSize);
    eResultCode SendPacket(PacketBaseC2S* packetData, DWORD packetSize);

    //
    eResultCode ReqAuth();
    eResultCode ReqEnter();
    eResultCode ReqMove(int posX, int posY);
    eResultCode ReqInteraction(int targetId, int type);

    eResultCode RepHeartBeat();
    eResultCode ReqEcho();
};

//
class CUserSessionMgr
{
private:
    DWORD _dwUserSessionIndex{0};
    int _nMaxUserSessionCount{0};
    std::list<CUserSession> _UserSessionList{};
    std::list<CUserSession*> _FreeUserSessionList{};

public:
    std::list<CUserSession*> _UsedUserSessionList{};
    Lock _Lock;

    //
private:
    CUserSessionMgr(int nMaxCount = 2000)
    {
        for (int cnt = 0; cnt < nMaxCount; ++cnt) {
            CUserSession* pData = new CUserSession(GetUniqueIndex());

            _UserSessionList.push_back(*pData);
            _FreeUserSessionList.push_back(pData);
        }

        _nMaxUserSessionCount = nMaxCount;
    }

    ~CUserSessionMgr()
    {
        _UserSessionList.clear();
        _FreeUserSessionList.clear();
        _UsedUserSessionList.clear();
    }

    DWORD GetUniqueIndex()
    {
        InterlockedIncrement((DWORD*)&_dwUserSessionIndex);
        return _dwUserSessionIndex;
    }

public:
    static CUserSessionMgr& GetInstance()
    {
        static CUserSessionMgr* pInstance = new CUserSessionMgr();
        return *pInstance;
    }

    CUserSession* GetFreeUserSession()
    {
        CUserSession* pRet = nullptr;

        {
            SafeLock lock(_Lock);

            if (false == _FreeUserSessionList.empty()) {
                pRet = _FreeUserSessionList.front();
                _FreeUserSessionList.pop_front();

                _UsedUserSessionList.push_back(pRet);
            }
        }

        return pRet;
    }

    void ReleaseUserSesssion(CUserSession* pUserSession)
    {
        {
            SafeLock lock(_Lock);

            if (std::find(_UsedUserSessionList.begin(), _UsedUserSessionList.end(), pUserSession) != _UsedUserSessionList.end()) {
                _UsedUserSessionList.remove(pUserSession);
                _FreeUserSessionList.push_back(pUserSession);
            } else {
            }
        }
    }

    void GetUserSessionList(std::list<CUserSession*>& userList)
    {
        userList.clear();

        SafeLock lock(_Lock);
        userList = _UsedUserSessionList;
    }
};

bool S2C_AuthResult(CUserSession* userSession, const SC_P_AUTH_RESULT& packet);
bool S2C_Enter(CUserSession* userSession, const SC_P_ENTER& packet);
bool S2C_Move(CUserSession* userSession, const SC_P_MOVE& packet);
bool S2C_Interaction(CUserSession* userSession, const SC_P_INTERACTION& packet);
bool S2C_Heartbeat(CUserSession* userSession, const SC_P_HEARTBEAT& packet);
bool S2C_Echo(CUserSession* userSession, const SC_P_ECHO& packet);

#endif //__USER_SESSION_H__