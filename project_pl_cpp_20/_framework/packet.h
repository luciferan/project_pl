#pragma once
#ifndef __PACKET_H__
#define __PACKET_H__

#include "./_common.h"
#include "./buffer.h"

//
static const DWORD PACKET_CHECK_HEAD_KEY{0x00000000};
static const DWORD PACKET_CHECK_TAIL_KEY{0x10000000};
static const DWORD MAX_PACKET_SIZE = (1024 * 10);

#pragma pack(push, 1)
struct PacketBase
{
    unsigned int type{0};

    virtual void SerializeHead(Serializer &ser)
    {
        ser.Value(type);
    }
    virtual void Serialize(Serializer &ser) {}
};
#pragma pack(pop)

//
struct PacketHead
{
    DWORD dwCheckHead{0};
    DWORD dwLength{0}; // = sizeof(Head) + sizeof(Body) + sizeof(Tail)
    DWORD dwProtocol{0};
};

struct PacketTail
{
    DWORD dwCheckTail{0};
};

//
eResultCode MakeNetworkPacket(DWORD IN dwProtocol, char IN* pSendData, DWORD IN dwSendDataSize, char OUT* pSendBuffer, DWORD IN OUT& dwSendBufferSize);
eResultCode ParseNetworkData(CircleBuffer IN& Buffer, DWORD OUT& dwPacketLength);

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
        unsigned int type = static_cast<unsigned int>(packet.type);
        PacketFunctorBase<T>* pf = new (&functors[type]) PacketFunctor<T, P>(func);
        _packetFunctors[type] = pf;
    }

    bool Execute(T* obj, char* data, DWORD length)
    {
        Serializer unpack(data, length);
        PacketBase base;
        base.SerializeHead(unpack);
        unsigned int type = base.type;

        if (_packetFunctors[type]) {
            return _packetFunctors[type]->Execute(obj, data, length);
        }
        return false;
    }
};

//
#endif //__PACKET_H__