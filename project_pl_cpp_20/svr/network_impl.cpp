#include "../_framework/network.h"
#include "../_framework/Connector.h"
#include "../_lib/Log.h"
#include "../_lib/util.h"

//
eResultCode Network::DoUpdate(INT64 biCurrTime)
{
	if( 0 == _biUpdateTime )
		_biUpdateTime = GetTimeMilliSec() + (MILLISEC_A_SEC * 15);

	//
	if( _biUpdateTime < biCurrTime )
	{
		_biUpdateTime = GetTimeMilliSec() + (MILLISEC_A_SEC * 15);

		//
		wstring wstrReport = {};
		wstrReport.append(L"ConnectorState: ");
		wstrReport.append(CConnectorMgr::GetInstance().GetStateReport());

		g_PerformanceLog.Write(wstrReport.c_str());
	}

	return eResultCode::RESULT_SUCC;
}