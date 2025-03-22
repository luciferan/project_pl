#include "stdafx.h"

#include "./user_session_mgr.h"

#include "../_lib/util_time.h"

//
void UserSessionMgr::SetReleaseObj(UserSession* userSession)
{
    SafeLock lock(_lock);
    _releaseList.emplace_back(userSession);
}

void UserSessionMgr::SetReleaseObj(SafeLock&, UserSession* userSession)
{
    _releaseList.emplace_back(userSession);
}

void UserSessionMgr::SwapReleaseList(list<UserSession*>& swapList)
{
    SafeLock lock(_lock);
    swapList.swap(_releaseList);
}

void UserSessionMgr::AddUserSessionMap(UserSession* userSession)
{
    SafeLock lock(_lock);
    _userMap.insert({userSession->GetToken(), userSession});
}

void UserSessionMgr::DelUserSessionMap(UserSession* userSession)
{
    SafeLock lock(_lock);
    DelUserSessionMap(lock, userSession->GetToken());
}

void UserSessionMgr::DelUserSessionMap(SafeLock& lock, UserSession* userSession)
{
    if (userSession) {
        DelUserSessionMap(lock, userSession->GetToken());
    }
}

void UserSessionMgr::DelUserSessionMap(SafeLock&, INT64 token)
{
    if (auto it = _userMap.find(token); it != _userMap.end()) {
        _userMap.erase(it);
    }
}

UserSession* UserSessionMgr::GetUserSession(INT64 token)
{
    SafeLock lock(_lock);
    if (auto it = _userMap.find(token); it != _userMap.end()) {
        return it->second;
    }

    return nullptr;
}

void UserSessionMgr::DoUpdate(INT64 currTime)
{
    if (_biUpdateTime < currTime) {
        { // userSession update
            SafeLock lock(_lock);
            for (auto it : _usedList) {
                it->DoUpdate(lock, currTime);
            }
        }

        //
        list<UserSession*> releaseList{};
        SwapReleaseList(releaseList);

        if (releaseList.size()) {
            for (auto it : releaseList) {
                it->Release();
            }

            for (auto it : releaseList) {
                DelUserSessionMap(it);
                SetFreeObject(it);
            }
        }
        _biUpdateTime = GetTimeMilliSec() + (MILLISEC_A_SEC * 5);
    }
}