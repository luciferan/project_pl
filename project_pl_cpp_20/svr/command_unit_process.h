#pragma once
#ifndef __COMMAND_UNIT_PROCESS_H__
#define __COMMAND_UNIT_PROCESS_H__

#include "../_lib/command_unit_base.h"
#include "../_lib/util.h"

#include "./packet_svr.h"
#include "./user_session_mgr.h"
#include "./zone_mgr.h"

#include <list>

using namespace std;

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
        : CommandUnitBase(eCommandUnitPriority::Normal)
        , _zoneId(zoneId)
    {
        //MakeNetworkPacket((DWORD)packetData->type, (char*)packetData, packetSize, _sendBuffer, _sendBufferSize);
        Serializer pack;
        packetData->SerializeHead(pack);
        packetData->Serialize(pack);
        MakeNetworkPacket(pack.GetBuffer(), pack.GetDataSize(), _sendBuffer, _sendBufferSize, 0);
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
    SendPacketToUser(INT64 token, PacketBaseS2C* packetData, DWORD packetSize)
        : CommandUnitBase(eCommandUnitPriority::Normal)
        , _token(token)
    {
        //MakeNetworkPacket((DWORD)packetData->type, (char*)packetData, packetSize, _sendBuffer, _sendBufferSize);
        Serializer pack;
        packetData->SerializeHead(pack);
        packetData->Serialize(pack);
        MakeNetworkPacket(pack.GetBuffer(), pack.GetDataSize(), _sendBuffer, _sendBufferSize, 0);
    }
    SendPacketToUser(INT64 token, char* sendBuffer, DWORD sendBufferSize)
        : CommandUnitBase(eCommandUnitPriority::Normal)
        , _token(token)
    {
        memcpy_s(_sendBuffer, MAX_PACKET_BUFFER_SIZE, sendBuffer, sendBufferSize);
        _sendBufferSize = sendBufferSize;
    }
    void Operator();
};

class EnterCharacter : public CommandUnitBase
{
private:
    INT64 _token{0};
    int _zoneId{0};
    int _posX{0};
    int _posY{0};

public:
    EnterCharacter(INT64 token, int zoneId, int posX, int posY)
        : CommandUnitBase(eCommandUnitPriority::Normal)
        , _zoneId(zoneId), _token(token), _posX(posX), _posY(posY)
    {
    }
    void Operator();
};

class LeaveCharacter : public CommandUnitBase
{
private:
    INT64 _token{0};
    int _zoneId{0};

public:
    LeaveCharacter(INT64 token, int zoneId)
        : CommandUnitBase(eCommandUnitPriority::Normal)
        , _zoneId(zoneId), _token(token)
    {
    }
    void Operator();
};

class SendCharacterList : public CommandUnitBase
{
private:
    INT64 _exceptCharacterToken{0};
    int _zoneId{0};

public:
    SendCharacterList(INT64 exceptCharacterToken, int zoneId)
        : CommandUnitBase(eCommandUnitPriority::Normal)
        , _exceptCharacterToken(exceptCharacterToken), _zoneId(zoneId)
    {
    }
    void Operator();
};

//
#endif //__COMMAND_UNIT_PROCESS_H__