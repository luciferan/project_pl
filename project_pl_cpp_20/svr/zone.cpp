#include "stdafx.h"
#include "./zone.h"
#include "./command_unit_process.h"
#include "./process.h"

void Zone::OnRegistCharacter(Character* character)
{
    SafeLock lock(_lock);
    _characterMap.insert({character->GetId(), character});
}

void Zone::OnUnregistCharacter(Character* character)
{
    SafeLock lock(_lock);
    auto it = _characterMap.find(character->GetId());
    if (it != _characterMap.end()) {
        _characterMap.erase(it);
    }
}

void Zone::PacketBroadcast(char* sendBuffer, DWORD sendBufferSize)
{
    SafeLock lock(_lock);
    for( auto it : _characterMap){
        INT64 token = it.second->GetToken();

        GetCmdQueue().Add(new SendPacketToUser(token, sendBuffer, sendBufferSize));
    }
}