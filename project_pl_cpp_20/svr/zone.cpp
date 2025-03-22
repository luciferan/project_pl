#include "stdafx.h"

#include "./zone.h"
#include "./process.h"
#include "./command_unit_process.h"

//
void Zone::OnRegistCharacter(Character* character)
{
    SafeLock lock(_lock);
    _characterMap.insert({character->GetToken(), character});
}

void Zone::OnUnregistCharacter(Character* character)
{
    SafeLock lock(_lock);
    auto it = _characterMap.find(character->GetToken());
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

size_t Zone::GetCharacterDbList(list<CharacterDbData>& characterDbList, INT64 exceptToken /*= 0*/)
{
    SafeLock lock(_lock);
    for (auto it : _characterMap) {
        if (it.first == exceptToken) {
            continue;
        }
        characterDbList.emplace_back(it.second->GetDbData());
    }

    return characterDbList.size();
}