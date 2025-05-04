#include "stdafx.h"

#include "./packet.h"
#include "./buffer.h"
#include "./_common.h"

//
eResultCode MakeNetworkPacket(DWORD dwProtocol, char* pSendData, DWORD dwSendDataSize, char* pSendBuffer, DWORD& dwSendBufferSize)
{
    PacketHead head;
    PacketTail tail;

    head.dwCheckHead = PACKET_CHECK_HEAD_KEY;
    head.dwLength = sizeof(PacketHead) + dwSendDataSize + sizeof(PacketTail);
    head.dwProtocol = dwProtocol;

    tail.dwCheckTail = PACKET_CHECK_TAIL_KEY;

    if (head.dwLength > dwSendBufferSize) {
        return eResultCode::fail;
    }

    int nPos = 0;
    memcpy(pSendBuffer + nPos, (char*)&head, sizeof(head)); nPos += sizeof(head);
    memcpy(pSendBuffer + nPos, (char*)pSendData, dwSendDataSize); nPos += dwSendDataSize;
    memcpy(pSendBuffer + nPos, (char*)&tail, sizeof(tail)); nPos += sizeof(tail);

    dwSendBufferSize = head.dwLength;

    //
    return eResultCode::succ;
};

eResultCode ParseNetworkData(CircleBuffer IN& Buffer, DWORD OUT& dwPacketLength)
{
    DWORD dwDataSize = Buffer.GetDataSize();

    if (sizeof(PacketHead) > dwDataSize) {
        return eResultCode::pending;
    }

    PacketHead Head;
    Buffer.Read((char*)&Head, sizeof(PacketHead));

    if (PACKET_CHECK_HEAD_KEY != Head.dwCheckHead) {
        return eResultCode::invalid_packet;
    }

    if (Head.dwLength > dwDataSize) {
        return eResultCode::pending;
    }

    dwPacketLength = Head.dwLength;
    return eResultCode::succ;
}