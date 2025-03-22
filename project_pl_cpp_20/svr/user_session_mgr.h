#pragma once
#ifndef __USER_SESSION_MGR_H__
#define __USER_SESSION_MGR_H__

#include "../_lib/object_pool_mgr.h"

#include "./user_session.h"

#include <list>
#include <map>

using namespace std;

//
class UserSessionMgr : public ObjectMgrBase<UserSession>
{
private:
    list<UserSession*> _releaseList{};
    map<INT64, UserSession*> _userMap{};

    INT64 _biUpdateTime{0};

    //
private:
    UserSessionMgr() {}

public:
    virtual ~UserSessionMgr() {}
    static UserSessionMgr& GetInstance()
    {
        static UserSessionMgr* pInstance = new UserSessionMgr;
        return *pInstance;
    }

    void SetReleaseObj(UserSession* userSession);
    void SetReleaseObj(SafeLock&, UserSession* userSession);

    void SwapReleaseList(list<UserSession*>& swapList);

    void AddUserSessionMap(UserSession* userSession);
    void DelUserSessionMap(UserSession* userSession);
    void DelUserSessionMap(SafeLock&, UserSession* userSession);
    void DelUserSessionMap(SafeLock&, INT64 token);

    UserSession* GetUserSession(INT64 token);

    void DoUpdate(INT64 currTime);
};

//
#endif //__USER_SESSION_MGR_H__