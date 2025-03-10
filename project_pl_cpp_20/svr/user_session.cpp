#include "stdafx.h"

#include "../_framework/Connector.h"
#include "../_framework/Packet.h"
#include "../_lib/util.h"

#include "./user_session.h"
#include "./user_session_mgr.h"
#include "./process.h"
#include "./command_unit_process.h"

#include <atomic>

//
CUserSession::CUserSession()
{
    static atomic<DWORD> unique_user_session_uid{0};
    _dwUid = ++unique_user_session_uid;
}

CUserSession::~CUserSession()
{
}

void CUserSession::Release()
{
    if (_nub.GetZoneId()) {
        SafeLock lock(_lock);
        INT64 token = _nub.GetToken();
        int zoneId = _nub.GetZoneId();

        GetCmdQueue().Add(new ZoneLeave(token, zoneId));
    }

    //
    _biHeartbeatTimer = 0;
    _nHeartbeatCount = 0;
    _biUpdateTime = 0;

    if (_pConnector) {
        _pConnector->SetParam(nullptr);
        _pConnector->TryRelease();
    }
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

eResultCode CUserSession::SendPacket(char* packetBuffer, DWORD packetSize)
{
    return _pConnector->AddSendData(packetBuffer, packetSize);
}

//void CUserSession::DoUpdate(INT64 biCurrTime)
//{
//    if (_biUpdateTime < biCurrTime) {
//        _biUpdateTime = biCurrTime + (MILLISEC_A_SEC * 5);
//
//        if (_pConnector == nullptr || !_pConnector->GetActive()) {
//            UserSessionMgr::GetInstance().SetReleaseObj(this);
//            _biUpdateTime = INT64_MAX;
//            return;
//        }
//
//        if (_biHeartbeatTimer < biCurrTime && _pConnector->GetActive()) {
//            _biHeartbeatTimer = biCurrTime + (MILLISEC_A_SEC * 10);
//            if (5 < IncHeartbeat()) {
//
//            }
//            SC_P_HEARTBEAT sendPacket;
//            SendPacket(&sendPacket, sizeof(sendPacket));
//        }
//    }
//}

void CUserSession::DoUpdate(SafeLock& mgrLock, INT64 biCurrTime)
{
    if (_biUpdateTime < biCurrTime) {
        _biUpdateTime = biCurrTime + (MILLISEC_A_SEC * 5);

        if (_pConnector == nullptr || !_pConnector->GetActive()) {
            UserSessionMgr::GetInstance().SetReleaseObj(mgrLock, this);
            _biUpdateTime = INT64_MAX;
            return;
        }

        if (_biHeartbeatTimer < biCurrTime && _pConnector->GetActive()) {
            _biHeartbeatTimer = biCurrTime + (MILLISEC_A_SEC * 10);
            if (5 < IncHeartbeat()) {

            }
            SC_P_HEARTBEAT sendPacket;
            SendPacket(&sendPacket, sizeof(sendPacket));
        }
    }
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
    case PacketTypeC2S::leave:
        C2S_LEAVE(this, *((CS_P_LEAVE*)pPacketData));
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
    userSession->_nub.SetId(reqId);
    userSession->_nub.SetToken(token);

    //
    SC_P_AUTH_RESULT sendPacket;
    sendPacket.SetToken(reqId, token);
    userSession->SendPacket(&sendPacket, sizeof(sendPacket));

    return true;
}

static bool C2S_Enter(CUserSession* userSession, const CS_P_ENTER& packet)
{
    int zoneId = 0;
    int posX = rand() % 1000;
    int posY = rand() % 1000;

    SafeLock lock(userSession->GetLock());

    //
    INT64 token = userSession->_nub.GetToken();
    userSession->_nub.SetPos(zoneId, posX, posY);

    GetCmdQueue().Add(new ZoneEnter(token, zoneId, posX, posY));

    //ZoneMgr::GetInstance().EnterCharacter(zoneId, userSession->GetCharacter());

    ////
    //SC_P_ENTER sendPacket;
    //sendPacket.SetPos(token, posX, posY);
    //userSession->SendPacket(&sendPacket, sizeof(sendPacket));

    return true;
}

static bool C2S_LEAVE(CUserSession* userSession, const CS_P_LEAVE& packet)
{
    SafeLock lock(userSession->GetLock());

    INT64 token = userSession->_nub.GetToken();
    int zoneId = userSession->_nub.GetZoneId();

    GetCmdQueue().Add(new ZoneLeave(token, zoneId));

    return true;
}

static bool C2S_Move(CUserSession* userSession, const CS_P_MOVE& packet)
{
    int posX = packet.x;
    int posY = packet.y;

    SafeLock lock(userSession->GetLock());

    //
    INT64 token = userSession->_nub.GetToken();
    userSession->_nub.SetPos(posX, posY);

    int zoneId = userSession->_nub.GetZoneId();

    //
    SC_P_MOVE sendPacket;
    sendPacket.SetPos(token, posX, posY);
    //userSession->SendPacket(&sendPacket, sizeof(sendPacket));
    GetCmdQueue().Add(new SendBroadcastToAllUser(zoneId, &sendPacket, sizeof(sendPacket)));

    return true;
}

static bool C2S_Heartbeat(CUserSession* userSession, const CS_P_HEARTBEAT& packet)
{
    userSession->DecHeartbeat();

    return true;
}

static bool C2S_Interaction(CUserSession* userSession, const CS_P_INTERACTION& packet)
{
    INT64 targetToken = packet.targetToken;
    int interactionType = packet.interactionType;

    SafeLock lock(userSession->GetLock());

    //
    INT64 token = userSession->_nub.GetToken();
    int zoneId = userSession->_nub.GetZoneId();

    //
    SC_P_INTERACTION sendPacket;
    sendPacket.SetInteraction(token, targetToken, interactionType);
    ///userSession->SendPacket(&sendPacket, sizeof(sendPacket));
    GetCmdQueue().Add(new SendBroadcastToAllUser(zoneId, &sendPacket, sizeof(sendPacket)));

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