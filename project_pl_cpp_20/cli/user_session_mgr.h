#pragma once
#ifndef __USER_SESSION_MGR_H__
#define __USER_SESSION_MGR_H__

#include "../_lib/object_pool_mgr.h"
#include "./user_session.h"

class UserSessionMgr : public ObjectPoolMgrBase<UserSession>
{
private:
public:
    UserSessionMgr(int maxCount = 1) 
        :ObjectPoolMgrBase(maxCount)
    {
    }
    virtual ~UserSessionMgr() {}

    static UserSessionMgr& GetInstance()
    {
        static UserSessionMgr* pInstance = new UserSessionMgr;
        return *pInstance;
    }
};

#endif //__USER_SESSION_MGR_H__