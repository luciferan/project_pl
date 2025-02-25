#pragma once
#ifndef __PACKET_PROTOCOL_BASE_H__
#define __PACKET_PROTOCOL_BASE_H__

#include <Windows.h>

//
enum class PacketType : unsigned short
{
    Invalid = 0,
    C2S,
    S2C,
    Max
};

#pragma pack(push, 1)

struct PacketBase
{
public:
    PacketType base{PacketType::Invalid};

    PacketBase(PacketType base) : base(base) {}
};

#pragma pack(pop)

//
#endif //__PACKET_PROTOCOL_BASE_H__