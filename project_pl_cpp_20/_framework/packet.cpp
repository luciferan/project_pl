#include "stdafx.h"

#include "./packet.h"
#include "./buffer.h"
#include "./_common.h"

//
eResultCode MakeNetworkPacket(char* pSendData, DWORD dwSendDataSize, char* pSendBuffer, DWORD& dwSendBufferSize, UINT32 packetSeq)
{
    PacketHead head;
    PacketTail tail;

    head.ui32CheckHead = PACKET_CHECK_HEAD_KEY;
    head.ui32Length = sizeof(PacketHead) + dwSendDataSize + sizeof(PacketTail);

    tail.ui32CheckTail = PACKET_CHECK_TAIL_KEY;
    tail.ui32PacketSeq = packetSeq;
    tail.ui32PacketTime = _time32(NULL);

    if (head.ui32Length > dwSendBufferSize) {
        return eResultCode::fail;
    }

    int nPos = 0;
    memcpy(pSendBuffer + nPos, (char*)&head, sizeof(head)); nPos += sizeof(head);
    memcpy(pSendBuffer + nPos, (char*)pSendData, dwSendDataSize); nPos += dwSendDataSize;
    memcpy(pSendBuffer + nPos, (char*)&tail, sizeof(tail)); nPos += sizeof(tail);

    dwSendBufferSize = head.ui32Length;

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

    if (PACKET_CHECK_HEAD_KEY != Head.ui32CheckHead) {
        return eResultCode::invalid_packet;
    }

    if (Head.ui32Length > dwDataSize) {
        return eResultCode::pending;
    }

    dwPacketLength = Head.ui32Length;
    return eResultCode::succ;
}