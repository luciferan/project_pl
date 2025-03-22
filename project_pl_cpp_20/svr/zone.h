#pragma once
#ifndef __ZONE_H__
#define __ZONE_H__

#include <Windows.h>

#include "../_lib/safeLock.h"
#include "./character.h"

#include <map>
#include <list>

using namespace std;

//
class Character;
class Zone
{
private:
    Lock _lock;

    int _id{0};
    map<INT64, Character*> _characterMap{};

public:
    Zone() {}
    Zone(const Zone&) = delete;
    Zone& operator=(Zone&) = delete;
    virtual ~Zone() {}

    void SetId(int id) { _id = id; }
    int GetId() { return _id; }

    void OnRegistCharacter(Character* character);
    void OnUnregistCharacter(Character* character);
    void OnUnregistCharacter(INT64 token);

    size_t GetCharacterCount() { return _characterMap.size(); }
    size_t GetCharacterDbList(list<CharacterDbData>& characterDbList, INT64 exceptToken = 0);

    void PacketBroadcast(char* sendBuffer, DWORD sendBufferSize);
};

//
#endif //__ZONE_H__