#include "../_framework/Connector.h"
#include "../_framework/Packet.h"
#include "../_framework/Packet_Protocol.h"
#include "../_lib/util.h"

#include "UserSession.h"

//
int CUserSession::Clear()
{
	//InterlockedExchange(&_dwUsed, 0);
	//InterlockedExchange(&_dwActive, 0);
	_biHeartBeat = 0;
	_biUpdateTime = 0;

	if( _pConnector )
		_pConnector->SetParam(nullptr);

	return 0;
}

int CUserSession::Release()
{
	// todo: 객체 정리 작업
	Clear();

	//InterlockedExchange(&_dwActive, 0);

	return 0;
}

int CUserSession::MessageProcess(char *pData, int nLen)
{
	if( sizeof(sPacketHead) > nLen )
		return -1;

	//
	sPacketHead *pHeader = (sPacketHead*)pData;
	DWORD dwPackethLength = pHeader->dwLength - sizeof(sPacketHead) - sizeof(sPacketTail);
	DWORD dwProtocol = pHeader->dwProtocol;

	char *pPacketData = (char*)(pHeader + 1);

	//
	switch( dwProtocol )
	{
	case P_ECHO:
		{
			Log(format("MessageProcess {}: {}", GetIndex(), dwProtocol));
			SendPacketData(dwProtocol, pPacketData, dwPackethLength);
		}
		break;
	default:
		break;
	}

	//
	return 0;
}

int CUserSession::DoUpdate(INT64 uiCurrTime)
{
	if( _biHeartBeat < uiCurrTime )
	{
		_biHeartBeat = uiCurrTime + (MILLISEC_A_SEC * 30);
	}

	//
	return 0;
}

//int CUserSession::SendPacket(DWORD dwProtocol, char *pPacket, DWORD dwPacketLength)
//{
//	if( !_pConnector )
//		return -1;
//
//	struct
//	{
//		DWORD dwLength = 0;
//		DWORD dwProtocol = 0;
//		char Data[MAX_PACKET_BUFFER_SIZE] = {};
//	} NetworkData;
//
//	NetworkData.dwLength = sizeof(DWORD) + sizeof(DWORD) + dwPacketLength;
//	NetworkData.dwProtocol = dwProtocol;
//	memcpy(NetworkData.Data, pPacket, dwPacketLength);
//
//	_pConnector->AddSendData((char*)&NetworkData, NetworkData.dwLength);
//	return 0;
//}


eResultCode CUserSession::SendPacketData(DWORD dwProtocol, char *pData, DWORD dwSendDataSize)
{
	Log("info: SendPacketData");

	//CNetworkBuffer SendBuffer;
	char SendBuffer[MAX_PACKET_BUFFER_SIZE] = {};
	DWORD dwSendBufferSize = sizeof(SendBuffer);
	MakeNetworkPacket(dwProtocol, pData, dwSendDataSize, (char*)&SendBuffer, dwSendBufferSize);

	return _pConnector->AddSendData((char*)&SendBuffer, dwSendBufferSize);
}