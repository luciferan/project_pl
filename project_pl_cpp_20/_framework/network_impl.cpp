#include "stdafx.h"

#include "./network.h"
#include "./connector_mgr.h"

#include "../_lib/Log.h"
#include "../_lib/util.h"

#include <string>

//
eResultCode Network::DoUpdate(INT64 biCurrTime)
{
    if (_biUpdateTime < biCurrTime) {
        _biUpdateTime = GetTimeMilliSec() + (MILLISEC_A_MIN);

        //
        wstring wstrReport = {};
        wstrReport.append(L"ConnectorState: ");
        wstrReport.append(ConnectorMgr::GetInstance().GetReport());

        g_PerformanceLog.Write(wstrReport.c_str());
    }

    return eResultCode::succ;
}