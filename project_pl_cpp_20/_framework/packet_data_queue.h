#pragma once
#ifndef __PACKET_DATA_QUEUE_H__
#define __PACKET_DATA_QUEUE_H__

#include "./_common.h"
#include "./buffer.h"

#include "../_lib/safe_lock.h"

#include <unordered_map>
#include <list>
#include <queue>

using namespace std;

//
enum ePacketDataQueue
{
    MAX_PACKET_QUEUE_PHASE = 4,
};

//
class PacketStruct
    : public BufferBase
{
public:
    Connector* _pConnector{nullptr};

public:
    PacketStruct() {};
    virtual ~PacketStruct() {};
};

//
class PacketDataQueue
{
protected:
    Lock _cs[ePacketDataQueue::MAX_PACKET_QUEUE_PHASE + 1];
    HANDLE _hQueueEvent[ePacketDataQueue::MAX_PACKET_QUEUE_PHASE + 1]{INVALID_HANDLE_VALUE,};

    list<PacketStruct*> lstPacketQueue[ePacketDataQueue::MAX_PACKET_QUEUE_PHASE + 1]{};

    //
public:
    PacketDataQueue()
    {
        for (auto idx = 0; idx < ePacketDataQueue::MAX_PACKET_QUEUE_PHASE + 1; ++idx) {
            _hQueueEvent[idx] = CreateEvent(0, 0, 0, 0);
            lstPacketQueue[idx].clear();
        }
    }
    virtual ~PacketDataQueue()
    {
        for (auto idx = 0; idx < ePacketDataQueue::MAX_PACKET_QUEUE_PHASE + 1; ++idx) {
            CloseHandle(_hQueueEvent[idx]);
            lstPacketQueue[idx].clear();
        }
    };

    void ForceActivateQueueEvent()
    {
        for (auto idx = 0; idx < ePacketDataQueue::MAX_PACKET_QUEUE_PHASE + 1; ++idx) {
            SetEvent(_hQueueEvent[idx]);
        }
    }

    size_t Push(PacketStruct* pPacketData, int phase)
    {
        if (0 > phase || ePacketDataQueue::MAX_PACKET_QUEUE_PHASE < phase) {
            return -1;
        }

        SafeLock lock(_cs[phase]);
        lstPacketQueue[phase].push_back(pPacketData);
        return lstPacketQueue[phase].size();
    }

    PacketStruct* Pop(int phase)
    {
        if (0 > phase || ePacketDataQueue::MAX_PACKET_QUEUE_PHASE < phase) {
            return nullptr;
        }
        if (lstPacketQueue[phase].empty()) {
            return nullptr;
        }

        PacketStruct* pData(nullptr);

        SafeLock lock(_cs[phase]);
        auto iter = lstPacketQueue[phase].begin();
        pData = *iter;
        lstPacketQueue[phase].pop_front();

        return pData;
    }
};

//
class RecvPacketQueue
    : public PacketDataQueue
{
private:
    RecvPacketQueue() {};
    ~RecvPacketQueue() {};

public:
    static RecvPacketQueue& GetInstance()
    {
        static RecvPacketQueue* pInstance = new RecvPacketQueue();
        return *pInstance;
    }

    size_t Push(PacketStruct* pPacketData, int phase = 0)
    {
        if (!pPacketData) {
            return -1;
        }
        if (0 > phase || ePacketDataQueue::MAX_PACKET_QUEUE_PHASE <= phase) {
            return -1;
        }

        auto nDataSize = PacketDataQueue::Push(pPacketData, phase);
        SetEvent(_hQueueEvent[phase]);
        return nDataSize;
    }

    PacketStruct* Pop(int phase = 0)
    {
        if (0 > phase || ePacketDataQueue::MAX_PACKET_QUEUE_PHASE <= phase) {
            return nullptr;
        }

        PacketStruct* pPacket = PacketDataQueue::Pop(phase);
        if (!pPacket) {
            WaitForSingleObject(_hQueueEvent[phase], INFINITE);
            pPacket = PacketDataQueue::Pop(phase);
        }
        return pPacket;
    }

    PacketStruct* GetFreePacketStruct()
    {
        PacketStruct* pPacket = new PacketStruct;
        if (!pPacket) {
            return nullptr;
        }

        return pPacket;
    }

    void ReleasePacketStruct(PacketStruct* pPacket)
    {
        if (pPacket) {
            delete pPacket;
            pPacket = nullptr;
        }
    }
};

//
class SendPacketQueue
    : public PacketDataQueue
{
private:
    SendPacketQueue() {}
    ~SendPacketQueue() {}

public:
    static SendPacketQueue& GetInstance()
    {
        static SendPacketQueue* pInstance = new SendPacketQueue();
        return *pInstance;
    }

    size_t Push(PacketStruct* pPacketData)
    {
        if (!pPacketData) {
            return -1;
        }
        auto nDataSize = PacketDataQueue::Push(pPacketData, 0);
        SetEvent(_hQueueEvent[0]);
        return nDataSize;
    }

    PacketStruct* Pop()
    {
        PacketStruct* pPacket = PacketDataQueue::Pop(0);
        if (!pPacket) {
            WaitForSingleObject(_hQueueEvent[0], INFINITE);
            pPacket = PacketDataQueue::Pop(0);
        }
        return pPacket;
    }
};

//
#endif //__PACKET_DATA_QUEUE_H__