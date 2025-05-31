#include "stdafx.h"

#include "../_framework/Connector.h"
#include "../_framework/Packet.h"
#include "../_lib/util.h"

#include "./user_session.h"
#include "./user_session_mgr.h"
#include "./process.h"
#include "./command_unit_process.h"

#include <iostream>
#include <atomic>

//
UserSession::HandlerImpl UserSession::_impl;

UserSession::UserSession()
{
    static atomic<DWORD> unique_user_session_uid{0};
    _dwUid = ++unique_user_session_uid;
}

UserSession::~UserSession()
{
}

void UserSession::Release()
{
    if (_nub.GetZoneId()) {
        SafeLock lock(_lock);
        INT64 token = _nub.GetToken();
        int zoneId = _nub.GetZoneId();

        GetCmdQueue().Add(new LeaveCharacter(token, zoneId));
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

eResultCode UserSession::SendPacket(PacketBaseS2C* packetData, DWORD packetSize)
{
    char sendBuffer[MAX_PACKET_BUFFER_SIZE]{};
    DWORD sendBufferSize = MAX_PACKET_BUFFER_SIZE;
    //MakeNetworkPacket((DWORD)packetData->type, (char*)packetData, packetSize, sendBuffer, sendBufferSize);
    //return _pConnector->AddSendData(sendBuffer, sendBufferSize);

    Serializer pack;
    packetData->SerializeHead(pack);
    packetData->Serialize(pack);
    MakeNetworkPacket((DWORD)packetData->type, pack.GetBuffer(), pack.GetDataSize(), sendBuffer, sendBufferSize);
    return _pConnector->AddSendData(sendBuffer, sendBufferSize);
}

eResultCode UserSession::SendPacket(char* packetBuffer, DWORD packetSize)
{
    return _pConnector->AddSendData(packetBuffer, packetSize);
}

void UserSession::DoUpdate(SafeLock& mgrLock, INT64 biCurrTime)
{
    SafeLock lock(_lock);

    if (_biUpdateTime < biCurrTime) {
        _biUpdateTime = biCurrTime + (MILLISEC_A_SEC * 5);

        if (_pConnector == nullptr || !_pConnector->GetActive()) {
            UserSessionMgr::GetInstance().SetReleaseObj(mgrLock, this);
            _biUpdateTime = INT64_MAX;
            return;
        }

        if (_biHeartbeatTimer < biCurrTime && _pConnector->GetActive()) {
            _biHeartbeatTimer = biCurrTime + (MILLISEC_A_SEC * 15);
            if (4 < IncHeartbeat()) {
                _biHeartbeatTimer = INT64_MAX;

                Log(format("heartbeat count over 5. disconnect. token {}", GetToken()));
                _pConnector->TryRelease();
                return;
            }

            SC_P_HEARTBEAT sendPacket;
            SendPacket(&sendPacket, sizeof(sendPacket));
        }
    }
}

bool UserSession::MessageProcess(char* pData, int nLen)
{
    if (sizeof(PacketHead) > nLen) {
        return false;
    }

    PacketHead* pHeader = (PacketHead*)pData;
    DWORD dwPackethLength = pHeader->dwLength - sizeof(PacketHead) - sizeof(PacketTail);
    DWORD dwProtocol = pHeader->dwProtocol;
    char* pPacketData = (char*)(pHeader + 1);

    return _impl.Execute(this, pPacketData, dwPackethLength);
}

static bool C2S_Auth(UserSession* userSession, const CS_P_AUTH& packet)
{
    CTimeSet now;

    int reqId = packet.id;
    INT64 token = (now.GetTime() * 10000 + GetUniqueSerialId());

    //
    SafeLock lock(userSession->GetLock());

    userSession->_nub.SetId(reqId);
    userSession->_nub.SetToken(token);

    UserSessionMgr::GetInstance().AddUserSessionMap(userSession);

    //
    SC_P_AUTH_RESULT sendPacket;
    sendPacket.SetToken(reqId, token);
    userSession->SendPacket(&sendPacket, sizeof(sendPacket));

    return true;
}

static bool C2S_Enter(UserSession* userSession, const CS_P_ENTER& packet)
{
    int zoneId = 1;
    int posX = rand() % 1000;
    int posY = rand() % 1000;

    //
    SafeLock lock(userSession->GetLock());

    INT64 token = userSession->_nub.GetToken();
    userSession->_nub.SetPos(zoneId, posX, posY);

    GetCmdQueue().Add(new EnterCharacter(token, zoneId, posX, posY));

    return true;
}

static bool C2S_LEAVE(UserSession* userSession, const CS_P_LEAVE& packet)
{
    SafeLock lock(userSession->GetLock());

    INT64 token = userSession->_nub.GetToken();
    int zoneId = userSession->_nub.GetZoneId();

    GetCmdQueue().Add(new LeaveCharacter(token, zoneId));

    return true;
}

static bool C2S_Move(UserSession* userSession, const CS_P_MOVE& packet)
{
    int posX = packet.x;
    int posY = packet.y;

    //
    SafeLock lock(userSession->GetLock());

    INT64 token = userSession->_nub.GetToken();
    userSession->_nub.SetPos(posX, posY);

    int zoneId = userSession->_nub.GetZoneId();

    //
    SC_P_MOVE sendPacket;
    sendPacket.SetPos(token, posX, posY);
    GetCmdQueue().Add(new SendBroadcastToAllUser(zoneId, &sendPacket, sizeof(sendPacket)));

    return true;
}

static bool C2S_Heartbeat(UserSession* userSession, const CS_P_HEARTBEAT& packet)
{
    SafeLock lock(userSession->GetLock());

    userSession->DecHeartbeat();
    cout << " <<< heartbeat:" << userSession->GetToken() << endl;

    return true;
}

static bool C2S_Interaction(UserSession* userSession, const CS_P_INTERACTION& packet)
{
    INT64 targetToken = packet.targetToken;
    int interactionType = packet.interactionType;

    //
    SafeLock lock(userSession->GetLock(), true);

    INT64 token = userSession->_nub.GetToken();
    int zoneId = userSession->_nub.GetZoneId();

    //
    SC_P_INTERACTION sendPacket;
    sendPacket.SetInteraction(token, targetToken, interactionType);
    GetCmdQueue().Add(new SendBroadcastToAllUser(zoneId, &sendPacket, sizeof(sendPacket)));

    return true;
}

static bool C2S_Echo(UserSession* userSession, const CS_P_ECHO& packet)
{
    SafeLock lock(userSession->GetLock(), true);

    INT64 token = userSession->_nub.GetToken();
    int zoneId = userSession->_nub.GetZoneId();

    cout << userSession->GetToken() << ": " << packet.echoData << endl;

    SC_P_ECHO sendPacket;
    sendPacket.SetData(token, packet.echoData, packet.echoDataSize);
    GetCmdQueue().Add(new SendBroadcastToAllUser(zoneId, &sendPacket, sizeof(sendPacket)));

    return true;
}

void RegisterPacketHandlers(UserSession::HandlerImpl &impl)
{
    impl.Register(&C2S_Auth);
    impl.Register(&C2S_Enter);
    impl.Register(&C2S_LEAVE);
    impl.Register(&C2S_Move);
    impl.Register(&C2S_Heartbeat);
    impl.Register(&C2S_Interaction);
    impl.Register(&C2S_Echo);
}
