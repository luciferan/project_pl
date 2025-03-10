#pragma once
#ifndef __PACKET_DATA_QUEUE_H__
#define __PACKET_DATA_QUEUE_H__

#include "./_common.h"

#include "../_lib/SafeLock.h"
#include "Buffer.h"
//#include "../_framework/Packet.h"

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
class CPacketStruct
    : public CBuffer
{
public:
    Connector* pConnector{nullptr};

public:
    CPacketStruct() {};
    virtual ~CPacketStruct() {};
};

//
class CPacketDataQueue
{
protected:
    Lock _cs[ePacketDataQueue::MAX_PACKET_QUEUE_PHASE + 1];
    HANDLE _hQueueEvent[ePacketDataQueue::MAX_PACKET_QUEUE_PHASE + 1]{INVALID_HANDLE_VALUE,};

    list<CPacketStruct*> lstPacketQueue[ePacketDataQueue::MAX_PACKET_QUEUE_PHASE + 1]{};

    //
public:
    CPacketDataQueue()
    {
        for (auto idx = 0; idx < ePacketDataQueue::MAX_PACKET_QUEUE_PHASE + 1; ++idx) {
            _hQueueEvent[idx] = CreateEvent(0, 0, 0, 0);
            lstPacketQueue[idx].clear();
        }
    }
    virtual ~CPacketDataQueue()
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

    size_t Push(CPacketStruct* pPacketData, int phase)
    {
        if (0 > phase || ePacketDataQueue::MAX_PACKET_QUEUE_PHASE < phase) {
            return -1;
        }

        SafeLock lock(_cs[phase]);
        lstPacketQueue[phase].push_back(pPacketData);
        return lstPacketQueue[phase].size();
    }

    CPacketStruct* Pop(int phase)
    {
        if (0 > phase || ePacketDataQueue::MAX_PACKET_QUEUE_PHASE < phase) {
            return nullptr;
        }
        if (lstPacketQueue[phase].empty()) {
            return nullptr;
        }

        CPacketStruct* pData(nullptr);

        SafeLock lock(_cs[phase]);
        auto iter = lstPacketQueue[phase].begin();
        pData = *iter;
        lstPacketQueue[phase].pop_front();

        return pData;
    }
};

//
class CRecvPacketQueue
    : public CPacketDataQueue
{
private:
    CRecvPacketQueue() {};
    ~CRecvPacketQueue() {};

public:
    static CRecvPacketQueue& GetInstance()
    {
        static CRecvPacketQueue* pInstance = new CRecvPacketQueue();
        return *pInstance;
    }

    size_t Push(CPacketStruct* pPacketData, int phase = 0)
    {
        if (!pPacketData) {

            return -1;
        }
        if (0 > phase || ePacketDataQueue::MAX_PACKET_QUEUE_PHASE <= phase) {
            return -1;
        }

        auto nDataSize = CPacketDataQueue::Push(pPacketData, phase);
        SetEvent(_hQueueEvent[phase]);
        return nDataSize;
    }

    CPacketStruct* Pop(int phase = 0)
    {
        if (0 > phase || ePacketDataQueue::MAX_PACKET_QUEUE_PHASE <= phase) {
            return nullptr;
        }

        CPacketStruct* pPacket = CPacketDataQueue::Pop(phase);
        if (!pPacket) {
            WaitForSingleObject(_hQueueEvent[phase], INFINITE);
            pPacket = CPacketDataQueue::Pop(phase);
        }
        return pPacket;
    }

    CPacketStruct* GetFreePacketStruct()
    {
        CPacketStruct* pPacket = new CPacketStruct;
        if (!pPacket) {
            return nullptr;
        }

        return pPacket;
    }

    void ReleasePacketStruct(CPacketStruct* pPacket)
    {
        if (pPacket) {
            delete pPacket;
            pPacket = nullptr;
        }
    }
};

//
class CSendPacketQueue
    : public CPacketDataQueue
{
private:
    CSendPacketQueue() {}
    ~CSendPacketQueue() {}

public:
    static CSendPacketQueue& GetInstance()
    {
        static CSendPacketQueue* pInstance = new CSendPacketQueue();
        return *pInstance;
    }

    size_t Push(CPacketStruct* pPacketData)
    {
        if (!pPacketData) {
            return -1;
        }
        auto nDataSize = CPacketDataQueue::Push(pPacketData, 0);
        SetEvent(_hQueueEvent[0]);
        return nDataSize;
    }

    CPacketStruct* Pop()
    {
        CPacketStruct* pPacket = CPacketDataQueue::Pop(0);
        if (!pPacket) {
            WaitForSingleObject(_hQueueEvent[0], INFINITE);
            pPacket = CPacketDataQueue::Pop(0);
        }
        return pPacket;
    }
};

//
#endif //__PACKET_DATA_QUEUE_H__