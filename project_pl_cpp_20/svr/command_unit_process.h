#pragma once
#ifndef __COMMAND_UNIT_PROCESS_H__
#define __COMMAND_UNIT_PROCESS_H__

//#include "../_framework/Connector.h"
#include "../_lib/util.h"
#include "../_lib/command_unit_base.h"

//#include "./process.h"
#include "./packet_svr.h"
#include "./user_session_mgr.h"
#include "./zone_mgr.h"

//
enum class EBroadcastType : short
{
    All,
    Zone,
};

class SendBroadcastToAllUser : public CommandUnitBase
{
private:
    int _zoneId{0};
    char _sendBuffer[MAX_PACKET_BUFFER_SIZE]{};
    DWORD _sendBufferSize{MAX_PACKET_BUFFER_SIZE};

    //
public:
    SendBroadcastToAllUser(int zoneId, PacketBaseS2C* packetData, DWORD packetSize)
        : CommandUnitBase(ECommandUnitPriority::Normal)
        , _zoneId(zoneId)
    {
        MakeNetworkPacket((DWORD)packetData->type, (char*)packetData, packetSize, _sendBuffer, _sendBufferSize);
    }
    void Operator();
};

class SendPacketToUser : public CommandUnitBase
{
private:
    INT64 _token{0};
    char _sendBuffer[MAX_PACKET_BUFFER_SIZE]{};
    DWORD _sendBufferSize{MAX_PACKET_BUFFER_SIZE};

public:
    SendPacketToUser(INT64 token, char* sendBuffer, DWORD sendBufferSize)
        : CommandUnitBase(ECommandUnitPriority::Normal)
        , _token(token)
    {
        memcpy_s(_sendBuffer, MAX_PACKET_BUFFER_SIZE, sendBuffer, sendBufferSize);
        _sendBufferSize = sendBufferSize;
    }
    void Operator();
};

class ZoneEnter : public CommandUnitBase
{
private:
    INT64 _token{0};
    int _zoneId{0};
    int _posX{0};
    int _posY{0};

public:
    ZoneEnter(INT64 token, int zoneId, int posX, int posY)
        : CommandUnitBase(ECommandUnitPriority::Normal)
        , _zoneId(zoneId), _token(token), _posX(posX), _posY(posY)
    {
    }
    void Operator();
};

class ZoneLeave : public CommandUnitBase
{
private:
    INT64 _token{0};
    int _zoneId{0};

public:
    ZoneLeave(INT64 token, int zoneId)
        : CommandUnitBase(ECommandUnitPriority::Normal)
        , _zoneId(zoneId), _token(token)
    {
    }
    void Operator();
};

#endif //__COMMAND_UNIT_PROCESS_H__