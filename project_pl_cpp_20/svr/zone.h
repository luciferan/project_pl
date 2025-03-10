#pragma once
#ifndef __ZONE_H__
#define __ZONE_H__

#include <Windows.h>

#include "../_lib/safeLock.h"

#include <map>

using namespace std;

//
class Character;
class Zone
{
private:
    Lock _lock;

    int _id{0};
    map<int, Character*> _characterMap{};

public:
    Zone() {}
    Zone(const Zone&) = delete;
    Zone& operator=(Zone&) = delete;
    virtual ~Zone() {}

    void SetId(int id) { _id = id; }
    int GetId() { return _id; }

    void OnRegistCharacter(Character* character);
    void OnUnregistCharacter(Character* character);
    size_t GetCharacterCount() { return _characterMap.size(); }

    void PacketBroadcast(char* sendBuffer, DWORD sendBufferSize);
};



//
#endif //__ZONE_H__