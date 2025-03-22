#include "stdafx.h"

#include "./command_unit_process.h"
#include "./process.h"

//
void SendBroadcastToAllUser::Operator()
{
    Zone* zone = ZoneMgr::GetInstance().GetZone(_zoneId);
    if (zone) {
        zone->PacketBroadcast(_sendBuffer, _sendBufferSize);
    }
}

void SendPacketToUser::Operator()
{
    UserSession* user = UserSessionMgr::GetInstance().GetUserSession(_token);
    if (!user) {
        return;
    }

    user->SendPacket(_sendBuffer, _sendBufferSize);
}

void EnterCharacter::Operator()
{
    UserSession* user = UserSessionMgr::GetInstance().GetUserSession(_token);
    if (!user) {
        return;
    }

    ZoneMgr::GetInstance().EnterCharacter(_zoneId, user->GetCharacter());

    SC_P_ENTER sendPacket;
    sendPacket.SetPos(_token, _posX, _posY);
    GetCmdQueue().Add(new SendBroadcastToAllUser(_zoneId, &sendPacket, sizeof(sendPacket)));

    GetCmdQueue().Add(new SendCharacterList(_token, _zoneId));
}

void LeaveCharacter::Operator()
{
    if (UserSession* user = UserSessionMgr::GetInstance().GetUserSession(_token)) {
        user->_nub.ResetPos();
    }

    ZoneMgr::GetInstance().LeaveCharacter(_zoneId, _token);

    SC_P_LEAVE sendPacket;
    sendPacket.SetToken(_token);
    GetCmdQueue().Add(new SendBroadcastToAllUser(_zoneId, &sendPacket, sizeof(sendPacket)));
}

void SendCharacterList::Operator()
{
    UserSession* user = UserSessionMgr::GetInstance().GetUserSession(_exceptCharacterToken);
    if (!user) {
        return;
    }

    Zone* zone = ZoneMgr::GetInstance().GetZone(_zoneId);
    if (!zone) {
        return;
    }

    list<CharacterDbData> characterDbList{};
    zone->GetCharacterDbList(characterDbList, _exceptCharacterToken);

    SC_P_ENTER_CHARACTER_LIST sendPacket;
    for (auto it : characterDbList) {
        if (sendPacket.SetData(it)) {
            GetCmdQueue().Add(new SendPacketToUser(_exceptCharacterToken, &sendPacket, sizeof(sendPacket)));
            sendPacket.Reset();
        }
    }

    if (0 < sendPacket.count) {
        GetCmdQueue().Add(new SendPacketToUser(_exceptCharacterToken, &sendPacket, sizeof(sendPacket)));
    }
}