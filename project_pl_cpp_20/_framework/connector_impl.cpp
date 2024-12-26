#include "./connector.h"
#include "./util.h"

//
int CConnector::DataParsing()
{
	int nPacketLength = 0;
	_RecvDataBuffer.Read((char*)&nPacketLength, sizeof(int));

	if( (int)_RecvDataBuffer.GetDataSize() < nPacketLength)
		return 0;

	return nPacketLength;
}

//int CConnector::DataParsing()
//{
//	int nPacketLength = 0;
//	_RecvDataBuffer.Read((char*)&nPacketLength, sizeof(int));
//	nPacketLength = htonl(nPacketLength);
//
//	if( _RecvDataBuffer.GetDataSize() < nPacketLength )
//		return 0;
//
//	return nPacketLength;
//}

bool CConnector::DoUpdate(INT64 biCurrTime)
{
	if( _biUpdateTimer < biCurrTime )
	{
		_biUpdateTimer = biCurrTime + MILLISEC_A_SEC;

		return true;
	}
	else
	{
		return false;
	}
}

bool CConnector::CheckHeartbeat(INT64 biCurrTime)
{
	if( GetActive() && _biHeartbeatTimer < biCurrTime )
	{
		_biHeartbeatTimer = biCurrTime + (MILLISEC_A_SEC * 30);

		return true;
	}
	else
	{
		return false;
	}
}

wstring CConnector::GetStateReport()
{
	wstring wstrReport = {};

	if( GetActive() )
		wstrReport.append(FormatW(L"[%d] connected: %d,", GetUniqueIndex(), GetSocket()));
	else
		wstrReport.append(FormatW(L"[%d] disconnected: ", GetUniqueIndex()));

	wstrReport.append(FormatW(L"sq:%d,", _SendQueue.size()));
	wstrReport.append(FormatW(L"rb:(%d/%d),", _RecvDataBuffer.GetDataSize(), _RecvDataBuffer.GetBufferSize()));
	wstrReport.append(FormatW(L"iq:%d", _InnerQueue.size()));

	return wstrReport;
}