#pragma once
#ifndef __PACKET_H__
#define __PACKET_H__

#include "./_common.h"

//#include "packet_Protocol_base.h"
#include "./buffer.h"

//
static const DWORD PACKET_CHECK_HEAD_KEY{0x00000000};
static const DWORD PACKET_CHECK_TAIL_KEY{0x10000000};

//
struct sPacketHead
{
    DWORD dwCheckHead{0};
    DWORD dwLength{0}; // = sizeof(Head) + sizeof(Body) + sizeof(Tail)
    DWORD dwProtocol{0};
};

struct sPacketBody
{
    char* pData{nullptr};
};

struct sPacketTail
{
    DWORD dwCheckTail{0};
};

//
DWORD MakeNetworkPacket(DWORD IN dwProtocol, char IN* pSendData, DWORD IN dwSendDataSize, char OUT* pSendBuffer, DWORD IN OUT& dwSendBufferSize);
DWORD ParseNetworkData(CCircleBuffer IN& Buffer, DWORD OUT& dwPacketLength);

//
#endif //__PACKET_H__