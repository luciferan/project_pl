#pragma once
#ifndef __CHARACTER_BASE_H__
#define __CHARACTER_BASE_H__

#include "stdafx.h"
//#include <windows.h>

#include <tuple>

using namespace std;

//
class Character
{
private:
    void* _param{nullptr};

    int _id{0};
    INT64 _token{0};

    int _zoneId{0};
    int _posX{0};
    int _posY{0};

public:
    Character() {}
    ~Character() {}

    void SetParam(void* param) { _param = param; }

    void SetId(int id) { _id = id; }
    const int GetId() { return _id; }
    void SetToken(INT64 token) { _token = token; }
    const INT64 GetToken() { return _token; }

    void SetPos(int zoneId, int x, int y) { _zoneId = zoneId; _posX = x; _posY = y; }
    void SetPos(int x, int y) { _posX = x; _posY = y; }
    const int GetZoneId() { return _zoneId; }
    const auto GetPos() { return tuple<int,int>(_posX, _posY); }
    const int GetPosX() { return _posX; }
    const int GetPosY() { return _posY; }
};

//
#endif //__CHARACTER_BASE_H__