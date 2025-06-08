#pragma once
#ifndef __PACKET_PROTOCOL_CS_H__
#define __PACKET_PROTOCOL_CS_H__

#include "./packet.h"

enum class PacketTypeC2S : UINT32
{
    invalid = 0,

    auth = 1,

    enter,
    leave,
    move,
    interaction,

    heartbeat,
    echo,
    Max,
};

#pragma pack(push, 1)
struct PacketBaseC2S : public PacketBase
{
public:
    PacketBaseC2S(PacketTypeC2S type)
    {
        this->ui32PacketType = static_cast<UINT32>(type);
    }
    void Serialize(Serializer& ser) {}
};

struct CS_P_AUTH : public PacketBaseC2S
{
public:
    INT32 id{0};

    CS_P_AUTH() : PacketBaseC2S(PacketTypeC2S::auth) {}
    void SetId(INT32 id)
    {
        this->id = id;
    }
    void Serialize(Serializer& ser)
    {
        ser.Value(id);
    }
};

struct CS_P_ENTER : public PacketBaseC2S
{
public:
    INT64 token{0};

    CS_P_ENTER() : PacketBaseC2S(PacketTypeC2S::enter) {}
    void SetToken(INT64 token)
    {
        this->token = token;
    }
    void Serialize(Serializer& ser)
    {
        ser.Value(token);
    }
};

struct CS_P_LEAVE : public PacketBaseC2S
{
public:
    INT64 token{0};

    CS_P_LEAVE() : PacketBaseC2S(PacketTypeC2S::leave) {}
    void SetToken(INT64 token)
    {
        this->token = token;
    }
    void Serialize(Serializer& ser)
    {
        ser.Value(token);
    }
};

struct CS_P_MOVE : public PacketBaseC2S
{
public:
    INT32 x{0}, y{0};

    CS_P_MOVE() : PacketBaseC2S(PacketTypeC2S::move) {}
    void SetPos(INT32 x, INT32 y)
    {
        this->x = x;
        this->y = y;
    }
    void Serialize(Serializer& ser)
    {
        ser.Value(x);
        ser.Value(y);
    }
};

struct CS_P_INTERACTION : public PacketBaseC2S
{
public:
    INT64 targetToken{0};
    INT32 interactionType{0};

    CS_P_INTERACTION() : PacketBaseC2S(PacketTypeC2S::interaction) {}
    void SetInteraction(INT64 targetToken, INT32 interactionType)
    {
        this->targetToken = targetToken;
        this->interactionType = interactionType;
    }
    void Serialize(Serializer& ser)
    {
        ser.Value(targetToken);
        ser.Value(interactionType);
    }
};

struct CS_P_HEARTBEAT : public PacketBaseC2S
{
public:
    CS_P_HEARTBEAT() : PacketBaseC2S(PacketTypeC2S::heartbeat) {}
    void Serialize(Serializer& ser) {}
};

struct CS_P_ECHO : public PacketBaseC2S
{
    static const int MAX_ECHO_DATA_LENGTH = 32;
public:
    INT32 echoDataSize{0};
    char echoData[MAX_ECHO_DATA_LENGTH]{};

    CS_P_ECHO() : PacketBaseC2S(PacketTypeC2S::echo) {}
    void SetData(const char* data, INT32 size)
    {
        echoDataSize = min(MAX_ECHO_DATA_LENGTH, size);
        memcpy_s(echoData, MAX_ECHO_DATA_LENGTH, data, size);
    }
    void Serialize(Serializer& ser)
    {
        ser.Value(echoDataSize);
        ser.Value(echoData, echoDataSize);
    }
};

#pragma pack(pop)

#endif //__PACKET_PROTOCOL_CS_H__