#pragma once
#ifndef __USER_SESSION_MGR_H__
#define __USER_SESSION_MGR_H__

#include "../_lib/object_pool_mgr.h"

#include "./user_session.h"

#include <list>
#include <map>

using namespace std;

//
class UserSessionMgr : public ObjectMgrBase<CUserSession>
{
private:
    list<CUserSession*> _releaseList{};
    map<INT64, CUserSession*> _userMap{};

    INT64 _biUpdateTime{0};

    //
private:
    //UserSessionMgr(int initCount = 20) :ObjectPoolMgrBase(initCount) { }
    UserSessionMgr() {}

public:
    virtual ~UserSessionMgr() {}
    static UserSessionMgr& GetInstance()
    {
        static UserSessionMgr* pInstance = new UserSessionMgr;
        return *pInstance;
    }

    void SetReleaseObj(CUserSession* userSession);
    void SetReleaseObj(SafeLock&, CUserSession* userSession);

    void SwapReleaseList(list<CUserSession*>& swapList);

    void AddUserSessionMap(CUserSession* userSession);
    void DelUserSessionMap(CUserSession* userSession);
    void DelUserSessionMap(SafeLock&, CUserSession* userSession);
    void DelUserSessionMap(SafeLock&, INT64 token);

    CUserSession* GetUserSession(INT64 token);

    void DoUpdate(INT64 currTime);
};

//
#endif //__USER_SESSION_MGR_H__