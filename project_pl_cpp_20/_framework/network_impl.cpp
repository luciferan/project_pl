#include "./network.h"
#include "./Connector.h"
#include "./Log.h"
#include "./util.h"

#include <string>

//
eResultCode Network::DoUpdate(INT64 biCurrTime)
{
	if( _biUpdateTime < biCurrTime )
	{
		_biUpdateTime = GetTimeMilliSec() + (MILLISEC_A_MIN);
		
		//
		wstring wstrReport = {};
		wstrReport.append(L"ConnectorState: ");
		wstrReport.append(CConnectorMgr::GetInstance().GetStateReport());

		g_PerformanceLog.Write(wstrReport.c_str());
	}

	return RESULT_SUCC;
}