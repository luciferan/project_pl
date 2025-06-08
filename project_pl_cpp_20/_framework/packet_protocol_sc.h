#pragma once
#ifndef __PACKET_PROTOCOL_SC_H__
#define __PACKET_PROTOCOL_SC_H__

#include "./packet.h"
#include "../_framework/character_base.h"

enum class PacketTypeS2C : UINT32
{
    invalid = 0,

    auth_result = 1,

    enter,
    enter_character,
    leave,
    move,
    interaction,

    heartbeat,
    echo,
    Max,
};

#pragma pack(push, 1)
struct PacketBaseS2C : public PacketBase
{
public:
    PacketBaseS2C(PacketTypeS2C type)
    {
        this->ui32PacketType = static_cast<UINT32>(type);
    }
    virtual void Serialize(Serializer& ser) {}
};

struct SC_P_AUTH_RESULT : public PacketBaseS2C
{
public:
    INT32 id{0};
    INT64 token{0};

    SC_P_AUTH_RESULT() : PacketBaseS2C(PacketTypeS2C::auth_result) {}
    void SetToken(INT32 id, INT64 token)
    {
        this->id = id;
        this->token = token;
    }
    void Serialize(Serializer& ser)
    {
        ser.Value(id);
        ser.Value(token);
    }
};

struct SC_P_ENTER : public PacketBaseS2C
{
public:
    INT64 token{0};
    INT32 x{0}, y{0};

    SC_P_ENTER() : PacketBaseS2C(PacketTypeS2C::enter) {}
    void SetPos(INT64 token, INT32 x, INT32 y)
    {
        this->token = token;
        this->x = x;
        this->y = y;
    }
    void Serialize(Serializer& ser)
    {
        ser.Value(token);
        ser.Value(x);
        ser.Value(y);
    }
};

struct SC_P_ENTER_CHARACTER_LIST : public PacketBaseS2C
{
    static const int MAX_SEND_COUNT_CHARACTER_DB_DATA = 20;
public:
    INT32 count{0};
    CharacterDbData data[MAX_SEND_COUNT_CHARACTER_DB_DATA]{};

    SC_P_ENTER_CHARACTER_LIST() : PacketBaseS2C(PacketTypeS2C::enter_character) {}
    void Reset()
    {
        count = 0;
        memset(data, 0, sizeof(data));

    }
    bool SetData(CharacterDbData& data)
    {
        this->data[this->count++] = data;
        return MAX_SEND_COUNT_CHARACTER_DB_DATA >= this->count;
    }
    void Serialize(Serializer& ser)
    {
        ser.Value(count);
        for (int idx = 0; idx < count; ++idx) {
            data[idx].Serialize(ser);
        }
    }
};

struct SC_P_LEAVE : public PacketBaseS2C
{
public:
    INT64 token{0};

    SC_P_LEAVE() : PacketBaseS2C(PacketTypeS2C::leave) {}
    void SetToken(INT64 token)
    {
        this->token = token;
    }
    void Serialize(Serializer& ser)
    {
        ser.Value(token);
    }
};

struct SC_P_MOVE : public PacketBaseS2C
{
public:
    INT64 token{0};
    INT32 x{0}, y{0};

    SC_P_MOVE() : PacketBaseS2C(PacketTypeS2C::move) {}
    void SetPos(INT64 token, INT32 x, INT32 y)
    {
        this->token = token;
        this->x = x;
        this->y = y;
    }
    void Serialize(Serializer& ser)
    {
        ser.Value(token);
        ser.Value(x);
        ser.Value(y);
    }
};

struct SC_P_INTERACTION : public PacketBaseS2C
{
public:
    INT64 token{0};
    INT64 targetToken{0};
    INT32 interactionType{0};

    SC_P_INTERACTION() : PacketBaseS2C(PacketTypeS2C::interaction) {}
    void SetInteraction(INT64 token, INT64 targetToken, INT32 type)
    {
        this->token = token;
        this->targetToken = targetToken;
        this->interactionType = type;
    }
    void Serialize(Serializer& ser)
    {
        ser.Value(token);
        ser.Value(targetToken);
        ser.Value(interactionType);
    }
};

struct SC_P_HEARTBEAT : public PacketBaseS2C
{
public:
    SC_P_HEARTBEAT() : PacketBaseS2C(PacketTypeS2C::heartbeat) {}
    void Serialize(Serializer& ser) {}
};

struct SC_P_ECHO : public PacketBaseS2C
{
    static const int MAX_ECHO_DATA_LENGTH = 32;
public:
    INT64 token{0};
    INT32 echoDataSize{0};
    char echoData[MAX_ECHO_DATA_LENGTH]{};

    SC_P_ECHO() : PacketBaseS2C(PacketTypeS2C::echo) {}
    void SetData(INT64 token, const char* data, INT32 size)
    {
        this->token = token;
        echoDataSize = min(MAX_ECHO_DATA_LENGTH, size);
        memcpy_s(echoData, MAX_ECHO_DATA_LENGTH, data, size);
    }
    void Serialize(Serializer& ser)
    {
        ser.Value(token);
        ser.Value(echoDataSize);
        ser.Value(echoData, echoDataSize);
    }
};

#pragma pack(pop)

#endif //__PACKET_PROTOCOL_SC_H__