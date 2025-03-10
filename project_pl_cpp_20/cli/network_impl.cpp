#include "../_framework/network.h"
#include "../_framework/Connector.h"
#include "../_lib/Log.h"
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

    //	g_PerformanceLog.Write(wstrReport.c_str());
    //}

    return eResultCode::RESULT_SUCC;
}