#pragma once
#ifndef __ZONE_MGR_H__
#define __ZONE_MGR_H__

#include "../_lib/object_pool_mgr.h"

#include "./zone.h"

using namespace std;

//
class ZoneMgr : public ObjectMgrBase<Zone>
{
private:
    map<int, Zone*> _zoneMap{};

private:
    ZoneMgr() {}

public:
    virtual ~ZoneMgr() {}
    static ZoneMgr& GetInstance()
    {
        static ZoneMgr* pInstance = new ZoneMgr;
        return *pInstance;
    }

    void EnterCharacter(int zoneId, Character* character)
    {
        SafeLock lock(_lock);
        if (auto it = _zoneMap.find(zoneId); it != _zoneMap.end()) {
            it->second->OnRegistCharacter(character);
        } else {
            auto obj(_objectPool.GetFreeObjectPtr());
            obj->SetId(zoneId);
            _zoneMap.insert({zoneId, obj});

            obj->OnRegistCharacter(character);
        }
    }

    void LeaveCharacter(int zoneId, Character* character)
    {
        SafeLock lock(_lock);
        if (auto it = _zoneMap.find(zoneId); it != _zoneMap.end()) {
            it->second->OnUnregistCharacter(character);
        }
    }

    void LeaveCharacter(int zoneId, INT64 token)
    {
        SafeLock lock(_lock);
        if (auto it = _zoneMap.find(zoneId); it != _zoneMap.end()) {
            it->second->OnUnregistCharacter(token);
        }
    }

    Zone* GetZone(int zoneId)
    {
        SafeLock lock(_lock);
        if (auto it = _zoneMap.find(zoneId); it != _zoneMap.end()) {
            return it->second;
        }
        return nullptr;
    }
};

//
#endif //__ZONE_MGR_H__