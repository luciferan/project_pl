#pragma once
#ifndef __CHARACTER_BASE_H__
#define __CHARACTER_BASE_H__

#include "stdafx.h"

#include <tuple>
#include <atomic>

using namespace std;

//
struct Position
{
    int zoneId{0};
    int posX{0};
    int posY{0};

    void ResetPos() { zoneId = 0; posX = 0; posY = 0; }

    void SetPos(int zoneId, int x, int y) { this->zoneId = zoneId; this->posX = x; this->posY = y; }
    void SetPos(int x, int y) { this->posX = x; this->posY = y; }
    const auto GetPos() { return tuple<int, int>(posX, posY); }
    const int GetZoneId() { return zoneId; }
    const int GetPosX() { return posX;}
    const int GetPosY() { return posY; }
};

struct CharacterDbData
{
    INT64 _token{0};
    Position _pos;
};

class Character
{
private:
    void* _param{nullptr};

    int _id{0};
    INT64 _token{0};

    Position _pos;

public:
    Character() {}
    ~Character() {}

    void SetParam(void* param) { _param = param; }

    void SetId(int id) { _id = id; }
    const int GetId() { return _id; }
    void SetToken(INT64 token) { _token = token; }
    const INT64 GetToken() { return _token; }

    void ResetPos() { _pos.ResetPos(); }
    void SetPos(int zoneId, int x, int y) { _pos.SetPos(zoneId, x, y); }
    void SetPos(int x, int y) { _pos.SetPos(x, y); }
    void SetPos(const Position& pos) { _pos = pos; }
    const auto GetPos() { return _pos.GetPos(); }
    const int GetZoneId() { return _pos.GetZoneId(); }
    const int GetPosX() { return _pos.GetPosX(); }
    const int GetPosY() { return _pos.GetPosY(); }

    CharacterDbData GetDbData() { return CharacterDbData{_token, _pos}; }
};

//
#endif //__CHARACTER_BASE_H__