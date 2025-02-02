#include "../_framework/Connector.h"
#include "../_framework/Packet.h"
#include "../_framework/Packet_Protocol.h"
#include "../_lib/util.h"

#include <string>
#include <format>

//
int CConnector::DataParsing()
{
	int nPacketLength = 0;

	DWORD dwRet = ParseNetworkData(_RecvDataBuffer, (DWORD&)nPacketLength);
	switch( dwRet )
	{
	case eResultCode::RESULT_INVALID_PACKET:
		return -1;
		break;
	}

	return nPacketLength;
}

//int CConnector::DataParsing()
//{
//	int nPacketLength = 0;
//
//	DWORD dwRet = ParseNetworkData(_RecvDataBuffer.GetBuffer(), _RecvDataBuffer.GetDataSize(), (DWORD&)nPacketLength);
//	switch( dwRet )
//	{
//	case eResultCode::RESULT_INVALID_PACKET:
//		return -1;
//		break;
//	}
//
//	return nPacketLength;
//}

//int CConnector::DataParsing()
//{
//	int nPacketLength = 0;
//	_RecvDataBuffer.Read((char*)&nPacketLength, sizeof(int));
//
//	if( _RecvDataBuffer.GetDataSize() < nPacketLength )
//		return 0;
//
//	return nPacketLength;
//}

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
	if( 0 == _biUpdateTimer )
		_biUpdateTimer = biCurrTime + MILLISEC_A_MIN;

	//
	if( _biUpdateTimer < biCurrTime )
	{
		_biUpdateTimer = biCurrTime + MILLISEC_A_MIN;

		g_PerformanceLog.Write(GetStateReport());

		//
		return true;
	}
	else
	{
		return false;
	}
}

bool CConnector::CheckHeartbeat(INT64 biCurrTime)
{
	if( 0 == _biHeartbeatTimer )
		_biHeartbeatTimer = biCurrTime + (MILLISEC_A_SEC * 30);

	//
	if( GetActive() && _biHeartbeatTimer < biCurrTime )
	{
		_biHeartbeatTimer = biCurrTime + (MILLISEC_A_SEC * 30);

		//struct 
		//{
		//	DWORD dwLength = 0;
		//	DWORD dwProtocol = 0;
		//	sP_HEARTBEAT Data;
		//} SendData;

		//SendData.dwLength = sizeof(SendData);
		//SendData.dwProtocol = P_HEARTBEAT;

		//AddSendData((char*)&SendData, sizeof(SendData));

		//
		sP_HEARTBEAT SendPacket;
		
		char SendBuffer[MAX_PACKET_BUFFER_SIZE] = {};
		DWORD dwSendBufferSize = sizeof(SendBuffer);
		MakeNetworkPacket(P_HEARTBEAT, (char*)&SendPacket, sizeof(SendPacket), (char*)&SendBuffer, dwSendBufferSize);

		//AddSendData((char*)&SendBuffer, dwSendBufferSize);

		//
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
		wstrReport.append(FormatW(L"[%d] connected: %d,", GetUID(), GetSocket()));
	else
		wstrReport.append(FormatW(L"[%d] disconnected: ", GetUID()));

	wstrReport.append(FormatW(L"sq:%d,", _SendQueue.size()));
	wstrReport.append(FormatW(L"rb:(%d/%d),", _RecvDataBuffer.GetDataSize(), _RecvDataBuffer.GetBufferSize()));
	wstrReport.append(FormatW(L"iq:%d", _InnerQueue.size()));

	return wstrReport;
}