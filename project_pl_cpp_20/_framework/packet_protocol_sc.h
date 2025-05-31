#pragma once
#ifndef __PACKET_PROTOCOL_SC_H__
#define __PACKET_PROTOCOL_SC_H__

#include "./packet.h"
#include "../_framework/character_base.h"

enum class PacketTypeS2C : unsigned int
{
    invalid = 0,
    auth_result,

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
        this->type = static_cast<unsigned int>(type);
    }
    virtual void Serialize(Serializer& ser) {}
};

struct SC_P_AUTH_RESULT : public PacketBaseS2C
{
public:
    int id{0};
    INT64 token{0};

    SC_P_AUTH_RESULT() : PacketBaseS2C(PacketTypeS2C::auth_result) {}
    void SetToken(int id, INT64 token)
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
    int x{0}, y{0};

    SC_P_ENTER() : PacketBaseS2C(PacketTypeS2C::enter) {}
    void SetPos(INT64 token, int x, int y)
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
    int count{0};
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
    int x{0}, y{0};

    SC_P_MOVE() : PacketBaseS2C(PacketTypeS2C::move) {}
    void SetPos(INT64 token, int x, int y)
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
    int interactionType{0};

    SC_P_INTERACTION() : PacketBaseS2C(PacketTypeS2C::interaction) {}
    void SetInteraction(INT64 token, INT64 targetToken, int type)
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
public:
    INT64 token{0};
    int echoDataSize{0};
    char echoData[32]{};

    SC_P_ECHO() : PacketBaseS2C(PacketTypeS2C::echo) {}
    void SetData(INT64 token, const char* data, int size)
    {
        this->token = token;
        echoDataSize = min(32, size);
        memcpy_s(echoData, 32, data, size);
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