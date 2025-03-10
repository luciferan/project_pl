#pragma once
#ifndef __CONNECTOR_MGR_H__
#define __CONNECTOR_MGR_H__

#include "./_common.h"

#include "../_lib/object_pool_mgr.h"
#include "../_lib/safeLock.h"

#include "./connector.h"

#include <string>
#include <format>

//
class Connector;
class ConnectorMgr : public ObjectPoolMgrBase<Connector>
{
private:
    ConnectorMgr(int initCount = 10)
        : ObjectPoolMgrBase(initCount)
    {
    }

public:
    virtual ~ConnectorMgr() {}

    static ConnectorMgr& GetInstance()
    {
        static ConnectorMgr* pInstance = new ConnectorMgr;
        return *pInstance;
    }
};

//
#endif //__CONNECTOR_MGR_H__