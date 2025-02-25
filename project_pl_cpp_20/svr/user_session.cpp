#include "../_framework/Connector.h"
#include "../_framework/Packet.h"
#include "../_lib/util.h"

#include "./user_session.h"

//
int CUserSession::Clear()
{
    _biHeartbeatTimer = 0;
    _nHeartbeatCount = 0;
    _biUpdateTime = 0;

    if (_pConnector) {
        _pConnector->SetParam(nullptr);
    }

    return 0;
}

int CUserSession::Release()
{
    Clear();
    return 0;
}

eResultCode CUserSession::SendPacketData(DWORD dwProtocol, char* pData, DWORD dwSendDataSize)
{
    Log("info: SendPacketData");

    //CNetworkBuffer SendBuffer;
    char SendBuffer[MAX_PACKET_BUFFER_SIZE]{};
    DWORD dwSendBufferSize{sizeof(SendBuffer)};
    MakeNetworkPacket(dwProtocol, pData, dwSendDataSize, (char*)&SendBuffer, dwSendBufferSize);

    return _pConnector->AddSendData((char*)&SendBuffer, dwSendBufferSize);
}

eResultCode CUserSession::SendPacket(PacketBaseS2C* packetData, DWORD packetSize)
{
    char sendBuffer[MAX_PACKET_BUFFER_SIZE]{};
    DWORD sendBufferSize = MAX_PACKET_BUFFER_SIZE;
    MakeNetworkPacket((DWORD)packetData->type, (char*)packetData, packetSize, sendBuffer, sendBufferSize);

    return _pConnector->AddSendData(sendBuffer, sendBufferSize);
}

int CUserSession::DoUpdate(INT64 uiCurrTime)
{
    if (_biHeartbeatTimer < uiCurrTime) {
        _biHeartbeatTimer = uiCurrTime + (MILLISEC_A_SEC * 30);
    }

    return 0;
}

bool CUserSession::MessageProcess(char* pData, int nLen)
{
    if (sizeof(sPacketHead) > nLen) {
        return false;
    }

    //
    sPacketHead* pHeader = (sPacketHead*)pData;
    DWORD dwPackethLength = pHeader->dwLength - sizeof(sPacketHead) - sizeof(sPacketTail);
    DWORD dwProtocol = pHeader->dwProtocol;

    char* pPacketData = (char*)(pHeader + 1);

    PacketTypeC2S packetType = (PacketTypeC2S)dwProtocol;
    switch (packetType) {
    case PacketTypeC2S::auth:
        C2S_Auth(this, *((CS_P_AUTH*)pPacketData));
        break;
    case PacketTypeC2S::enter:
        C2S_Enter(this, *((CS_P_ENTER*)pPacketData));
        break;
    case PacketTypeC2S::move:
        C2S_Move(this, *((CS_P_MOVE*)pPacketData));
        break;
    case PacketTypeC2S::interaction:
        C2S_Interaction(this, *((CS_P_INTERACTION*)pPacketData));
        break;
    case PacketTypeC2S::heartbeat:
        C2S_Heartbeat(this, *((CS_P_HEARTBEAT*)pPacketData));
        break;
    case PacketTypeC2S::echo:
        C2S_Echo(this, *((CS_P_ECHO*)pPacketData));
        break;
    default:
        LogError(format("invalid packet protocol {}", dwProtocol));
        break;
    }

    //
    return 0;
}

static bool C2S_Auth(CUserSession* userSession, const CS_P_AUTH& packet)
{
    CTimeSet now;

    int reqId = packet.id;
    INT64 token = (now.GetTime() * 10000 + GetUniqueSerialId());

    SafeLock lock(userSession->GetLock());

    //
    userSession->_nub.id = reqId;
    userSession->_nub.token = token;

    //
    SC_P_AUTH_RESULT sendPacket;
    sendPacket.SetToken(reqId, token);
    userSession->SendPacket(&sendPacket, sizeof(sendPacket));

    return true;
}

static bool C2S_Enter(CUserSession* userSession, const CS_P_ENTER& packet)
{
    int posX = rand() % 1000;
    int posY = rand() % 1000;

    SafeLock lock(userSession->GetLock());

    //
    INT64 token = userSession->_nub.token;
    userSession->_nub.pos_x = rand() % 1000;
    userSession->_nub.pos_y = rand() % 1000;

    //
    SC_P_ENTER sendPacket;
    sendPacket.SetPos(token, posX, posY);
    userSession->SendPacket(&sendPacket, sizeof(sendPacket));

    return true;
}

static bool C2S_Move(CUserSession* userSession, const CS_P_MOVE& packet)
{
    int posX = packet.x;
    int posY = packet.y;

    SafeLock lock(userSession->GetLock());

    //
    INT64 token = userSession->_nub.token;
    userSession->_nub.pos_x = posX;
    userSession->_nub.pos_y = posY;

    //
    SC_P_MOVE sendPacket;
    sendPacket.SetPos(token, posX, posY);
    userSession->SendPacket(&sendPacket, sizeof(sendPacket));

    return true;
}

static bool C2S_Heartbeat(CUserSession* userSession, const CS_P_HEARTBEAT& packet)
{
    userSession->DecHeartbeat();

    SafeLock lock(userSession->GetLock());
    if (userSession->GetHeartbeat() < 5) {
        return true;
    }

    //userSession->Disconnect();
    return true;
}

static bool C2S_Interaction(CUserSession* userSession, const CS_P_INTERACTION& packet)
{
    INT64 targetToken = packet.targetToken;
    int interactionType = packet.interactionType;

    SafeLock lock(userSession->GetLock());

    //
    INT64 token = userSession->_nub.token;

    //
    SC_P_INTERACTION sendPacket;
    sendPacket.SetInteraction(token, targetToken, interactionType);
    userSession->SendPacket(&sendPacket, sizeof(sendPacket));

    return true;
}

#include <iostream>
static bool C2S_Echo(CUserSession* userSession, const CS_P_ECHO& packet)
{
    cout << packet.echoData << endl;

    SC_P_ECHO sendPacket;
    sendPacket.SetData(packet.echoData, packet.echoDataSize);
    userSession->SendPacket(&sendPacket, sizeof(sendPacket));

    return true;
}