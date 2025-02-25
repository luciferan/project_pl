#pragma once
#ifndef __USER_SESSION_H__
#define __USER_SESSION_H__

#include <Windows.h>

#include "../_framework/_common.h"
#include "../_framework/packet_svr.h"
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
    Lock _lock;
    DWORD _dwIndex{0};
    CConnector* _pConnector{nullptr};

    INT64 _biHeartbeatTimer{0};
    unsigned int _nHeartbeatCount{0};
    INT64 _biUpdateTime{0};

public:
    Character _nub;

public:
    bool _bAuthed{false};

    //
public:
    CUserSession(DWORD dwIndex = 0)
        :_dwIndex(dwIndex)
    {
    }
    virtual ~CUserSession() {}

    int Clear();
    int Release();

    //
    DWORD GetIndex() { return _dwIndex; }
    bool GetActive() { return _bAuthed; }
    bool GetUsed() { return (nullptr != _pConnector); }

    void SetConnector(CConnector* pConnector) { _pConnector = pConnector; }
    CConnector* GetConnector() { return _pConnector; }

    void SetUpdateTime(INT64 biCurrTime) { _biUpdateTime = biCurrTime; }
    INT64 GetUpdateTime() { return _biUpdateTime; }
    bool CheckUpdateTime(INT64 biCurrTime) { return (_biUpdateTime > biCurrTime); }

    Lock& GetLock() { return _lock; }

    INT64 GetHeartbeatTimer() { return _biHeartbeatTimer; }
    void SetHeartbeatTimer(INT64 biCurrTime) { _biHeartbeatTimer = biCurrTime; }
    int GetHeartbeat() { return _nHeartbeatCount;  }
    int IncHeartbeat() { return InterlockedIncrement(&_nHeartbeatCount); }
    int DecHeartbeat() { return InterlockedDecrement(&_nHeartbeatCount); }

    //
    eResultCode SendPacketData(DWORD dwProtocol, char* pData, DWORD dwDataSize);
    eResultCode SendPacket(PacketBaseS2C* packetData, DWORD packetSize);

    //
    int DoUpdate(INT64 uiCurrTime);
    bool MessageProcess(char* pData, int nLen);
};

// CUserSessionMgr --------------------------------------------------------------
class CUserSessionMgr
{
private:
    DWORD _dwUserSessionIndex{0};
    int _nMaxUserSessionCount{0};
    list<CUserSession> _UserSessionList{};
    list<CUserSession*> _FreeUserSessionList{};

public:
    list<CUserSession*> _UsedUserSessionList{};
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

    virtual ~CUserSessionMgr()
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
        CUserSession* pRet{nullptr};

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
        //pUserSession->Release();
        {
            SafeLock lock(_Lock);

            if (std::find(_UsedUserSessionList.begin(), _UsedUserSessionList.end(), pUserSession) != _UsedUserSessionList.end()) {
                //InterlockedExchange(&pUserSession->_dwUsed, 0);
                //InterlockedExchange(&pUserSession->_dwActive, 0);

                _UsedUserSessionList.remove(pUserSession);
                _FreeUserSessionList.push_back(pUserSession);
            }
        }
    }
};


static bool C2S_Auth(CUserSession* userSession, const CS_P_AUTH& packet);
static bool C2S_Enter(CUserSession* userSession, const CS_P_ENTER& packet);
static bool C2S_Move(CUserSession* userSession, const CS_P_MOVE& packet);
static bool C2S_Heartbeat(CUserSession* userSession, const CS_P_HEARTBEAT& packet);
static bool C2S_Interaction(CUserSession* userSession, const CS_P_INTERACTION& packet);
static bool C2S_Echo(CUserSession* userSession, const CS_P_ECHO& packet);

#endif //__USER_SESSION_H__