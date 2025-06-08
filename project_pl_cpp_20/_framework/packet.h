#pragma once
#ifndef __PACKET_H__
#define __PACKET_H__

#include "./_common.h"
#include "./buffer.h"

//
static const UINT32 PACKET_CHECK_HEAD_KEY{0x00000000};
static const UINT32 PACKET_CHECK_TAIL_KEY{0x10000000};
static const UINT32 MAX_PACKET_SIZE{1024 * 10};

//
struct PacketHead
{
    UINT32 ui32CheckHead{0};
    UINT32 ui32Length{0}; // = sizeof(Head) + sizeof(Body) + sizeof(Tail)
};

struct PacketBody
{
    UINT32 ui32PacketType{0};

    virtual void SerializeHead(Serializer& ser)
    {
        ser.Value(ui32PacketType);
    }
    virtual void Serialize(Serializer& ser) {};
};
using PacketBase = PacketBody;

struct PacketTail
{
    UINT32 ui32CheckTail{0};
    UINT32 ui32PacketSeq;
    UINT32 ui32PacketTime;
};

//
eResultCode MakeNetworkPacket(char* pSendData, DWORD dwSendDataSize, char OUT* pSendBuffer, DWORD IN OUT& dwSendBufferSize, UINT32 packetSeq);
eResultCode ParseNetworkData(CircleBuffer& Buffer, DWORD OUT& dwPacketLength);

// packet handler ---------------------------------------------------------------
template <class T>
class PacketFunctorBase
{
public:
    virtual ~PacketFunctorBase() {}
    virtual bool Execute(T* obj, char* data, DWORD length) = 0;
};

template <class T, class P>
class PacketFunctor : public PacketFunctorBase<T>
{
private:
    typedef bool (*PacketFunc)(T*, const P&);
    PacketFunc _func;

public:
    PacketFunctor(PacketFunc func = 0) : _func(func) {}
    virtual ~PacketFunctor() {}
    virtual bool Execute(T* obj, const P& packet)
    {
        return (*_func)(obj, packet);
    }
    virtual bool Execute(T* obj, char* data, DWORD length)
    {
        Serializer unpack(data, length);
        P packet;
        packet.SerializeHead(unpack);
        packet.Serialize(unpack);
        return Execute(obj, packet);
    }
};

template <class T, int PACKET_MAX>
class PacketHandler
{
    template <class T1, int PM> friend void RegisterPacketHandlers(PacketHandler<T1, PM>& impl);

protected:
    PacketFunctorBase<T>* _packetFunctors[PACKET_MAX];

public:
    explicit PacketHandler()
    {
        RegisterPacketHandlers(*this);
    }
    virtual ~PacketHandler()
    {
    }

    template <class P>
    void Register(bool (*func)(T*, const P&))
    {
        typedef bool (*PacketFunc)(T*, const P&);

        static PacketFunctor<T, P> functors[PACKET_MAX];
        P packet;
        unsigned int type = static_cast<unsigned int>(packet.ui32PacketType);
        PacketFunctorBase<T>* pf = new (&functors[type]) PacketFunctor<T, P>(func);
        _packetFunctors[type] = pf;
    }

    bool Execute(T* obj, char* data, DWORD length)
    {
        Serializer unpack(data, length);
        PacketBase base;
        base.SerializeHead(unpack);
        unsigned int type = base.ui32PacketType;

        if (_packetFunctors[type]) {
            return _packetFunctors[type]->Execute(obj, data, length);
        }
        return false;
    }
};

//
#endif //__PACKET_H__