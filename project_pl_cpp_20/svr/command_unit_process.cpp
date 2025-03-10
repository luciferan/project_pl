#include "stdafx.h"
#include "./command_unit_process.h"
#include "./process.h"


void SendBroadcastToAllUser::Operator()
{
    Zone* zone = ZoneMgr::GetInstance().GetZone(_zoneId);
    if (zone) {
        zone->PacketBroadcast(_sendBuffer, _sendBufferSize);
    }
}

void SendPacketToUser::Operator()
{
    CUserSession* user = UserSessionMgr::GetInstance().GetUserSession(_token);
    if (!user) {
        return;
    }

    user->SendPacket(_sendBuffer, _sendBufferSize);
}

void ZoneEnter::Operator()
{
    CUserSession* user = UserSessionMgr::GetInstance().GetUserSession(_token);
    if (!user) {
        return;
    }

    ZoneMgr::GetInstance().EnterCharacter(_zoneId, user->GetCharacter());

    SC_P_ENTER sendPacket;
    sendPacket.SetPos(_token, _posX, _posY);
    GetCmdQueue().Add(new SendBroadcastToAllUser(_zoneId, &sendPacket, sizeof(sendPacket)));
}

void ZoneLeave::Operator()
{
    CUserSession* user = UserSessionMgr::GetInstance().GetUserSession(_token);
    if (user) {
        ZoneMgr::GetInstance().LeaveCharacter(_zoneId, user->GetCharacter());
    }

    SC_P_LEAVE sendPacket;
    sendPacket.SetToken(_token);
    GetCmdQueue().Add(new SendBroadcastToAllUser(_zoneId, &sendPacket, sizeof(sendPacket)));
}