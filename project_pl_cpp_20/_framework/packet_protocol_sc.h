#pragma once
#ifndef __PACKET_PROTOCOL_SC_H__
#define __PACKET_PROTOCOL_SC_H__

#include "./packet_protocol_base.h"

enum class PacketTypeS2C : unsigned int
{
    invalid = 0,
    auth_result,

    enter,
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
    PacketTypeS2C type{PacketTypeS2C::Max};

    PacketBaseS2C(PacketTypeS2C type) : PacketBase(PacketType::S2C), type(type) {}
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
};

struct SC_P_INTERACTION : public PacketBaseS2C
{
public:
    INT64 token{0};
    INT64 targetToken{0};
    int type{0};

    SC_P_INTERACTION() : PacketBaseS2C(PacketTypeS2C::interaction) {}
    void SetInteraction(INT64 token, INT64 targetToken, int type)
    {
        this->token = token;
        this->targetToken = targetToken;
        this->type = type;
    }
};

struct SC_P_HEARTBEAT : public PacketBaseS2C
{
public:
    SC_P_HEARTBEAT() : PacketBaseS2C(PacketTypeS2C::heartbeat) {}
};

struct SC_P_ECHO : public PacketBaseS2C
{
public:
    char echoData[32]{};
    int echoDataSize = 0;

    SC_P_ECHO() : PacketBaseS2C(PacketTypeS2C::echo) {}
    void SetData(const char* data, int size)
    {
        memcpy_s(echoData, 32, data, size);
        echoDataSize = min(32, size);
    }
};

#pragma pack(pop)

#endif //__PACKET_PROTOCOL_SC_H__