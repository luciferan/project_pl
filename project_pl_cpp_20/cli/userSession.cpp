#include "../_framework/Connector.h"
#include "../_framework/util.h"
#include "../_framework/Packet.h"
#include "../_framework/Packet_Protocol.h"

#include "UserSession.h"

//
int CUserSession::clear()
{
	InterlockedExchange(&_dwUsed, 0);
	InterlockedExchange(&_dwActive, 0);
	_uiHeartBeat = 0;

	_pConnector->SetParam(nullptr);

	return 0;
}

int CUserSession::MessageProcess(char *pData, int nLen)
{
	if( sizeof(sPacketHead) > nLen )
		return -1;

	sPacketHead *pHeader = (sPacketHead*)pData;
	DWORD dwPackethLength = pHeader->dwLength - sizeof(sPacketHead) - sizeof(sPacketTail);
	DWORD dwProtocol = pHeader->dwProtocol;

	char *pPacketData = (char*)(pHeader + 1);

	//
	//switch( dwProtocol )
	//{
	//default:
	//	break;
	//}

	//
	return 0;
}

int CUserSession::DoUpdate(INT64 uiCurrTime)
{
	if( _uiHeartBeat < uiCurrTime )
	{
		_uiHeartBeat = uiCurrTime;
	}

	//
	return 0;
}

eResultCode CUserSession::SendPacketData(DWORD dwProtocol, char *pData, DWORD dwSendDataSize)
{
	//CNetworkBuffer SendBuffer;
	char SendBuffer[MAX_PACKET_BUFFER_SIZE] = {};
	DWORD dwSendBufferSize = sizeof(SendBuffer);
	MakeNetworkPacket(dwProtocol, pData, dwSendDataSize, (char*)&SendBuffer, dwSendBufferSize);

	return _pConnector->AddSendData((char*)&SendBuffer, dwSendBufferSize);
}

//
eResultCode CUserSession::ReqAuth()
{
	sP_AUTH SendPacket;

	//
	return SendPacketData(P_AUTH, (char*)&SendPacket, sizeof(SendPacket));
}

eResultCode CUserSession::ReqEcho()
{
	sP_ECHO SendPacket;

	strncpy_s(SendPacket.echoData, "hello.", _countof(SendPacket.echoData));

	//
	return SendPacketData(P_ECHO, (char*)&SendPacket, sizeof(SendPacket));
}

eResultCode CUserSession::RepHeartBeat()
{
	sP_HEARTBEAT SendPacket;

	//
	return SendPacketData(P_HEARTBEAT, (char*)&SendPacket, sizeof(SendPacket));
}