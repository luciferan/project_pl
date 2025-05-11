#include "stdafx.h"

#include "../_framework/network.h"
#include "../_framework/connector.h"
#include "../_lib/log.h"
#include "../_lib/util.h"

//
eResultCode Network::DoUpdate(INT64 biCurrTime)
{
    //if( _biUpdateTime < biCurrTime )
    //{
    //	_biUpdateTime = GetTimeMilliSec() + (MILLISEC_A_MIN);

    //	//
    //	wstring wstrReport = {};
    //	wstrReport.append(L"ConnectorState: ");
    //	wstrReport.append(ConnectorMgr::GetInstance().GetReport());

    //	PerformanceLog(wstrReport);
    //}

    return eResultCode::succ;
}