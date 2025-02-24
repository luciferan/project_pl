#pragma once
#ifndef __USERSESSION_H__
#define __USERSESSION_H__

//
#include <Windows.h>

#include <list>
#include <queue>

#include "../_lib/safeLock.h"

using namespace std;

//
class CConnector;

//
class CUserSession
{
private:
    DWORD _dwIndex{0};

    CConnector* _pConnector{nullptr};

    INT64 _biHeartBeat{0};
    INT64 _biUpdateTime{0};

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

    //
    int MessageProcess(char* pData, int nLen);
    int DoUpdate(INT64 uiCurrTime);

    eResultCode SendPacketData(DWORD dwProtocol, char* pData, DWORD dwDataSize);

    //
    eResultCode RepAuth();
    eResultCode RepEcho();
};

//
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

#endif //__USERSESSION_H__