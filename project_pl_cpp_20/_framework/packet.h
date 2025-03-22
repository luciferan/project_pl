#pragma once
#ifndef __PACKET_H__
#define __PACKET_H__

#include "./_common.h"
#include "./buffer.h"

//
static const DWORD PACKET_CHECK_HEAD_KEY{0x00000000};
static const DWORD PACKET_CHECK_TAIL_KEY{0x10000000};

#pragma pack(push, 1)
struct PacketBase
{
public:
    unsigned int type{0};
};
#pragma pack(pop)

//
struct sPacketHead
{
    DWORD dwCheckHead{0};
    DWORD dwLength{0}; // = sizeof(Head) + sizeof(Body) + sizeof(Tail)
    DWORD dwProtocol{0};
};

struct sPacketTail
{
    DWORD dwCheckTail{0};
};

//
eResultCode MakeNetworkPacket(DWORD IN dwProtocol, char IN* pSendData, DWORD IN dwSendDataSize, char OUT* pSendBuffer, DWORD IN OUT& dwSendBufferSize);
eResultCode ParseNetworkData(CCircleBuffer IN& Buffer, DWORD OUT& dwPacketLength);

// packet handler ---------------------------------------------------------------
template <class T, class PT>
class PacketFunctorBase
{
public:
    virtual ~PacketFunctorBase() {}
    virtual bool Execute(T* obj, char* data) = 0;
};

template <class T, class PT, class P>
class PacketFunctor : public PacketFunctorBase<T, PT>
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
    virtual bool Execute(T* obj, char* data)
    {
        return Execute(obj, *((P*)data));
    }
};

template <class T, class PT, int PACKET_MAX>
class PacketHandler
{
    typedef PacketHandler<T, PT, PACKET_MAX> PacketHandlerType;
    template <class T1, class PT1, int PM> friend void RegisterPacketHandlers(PacketHandler<T1, PT1, PM>& impl);

protected:
    PacketFunctorBase<T, PT>* _packetFunctors[PACKET_MAX];

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

        static PacketFunctor<T, PT, P> functors[PACKET_MAX];
        P packet;
        unsigned int type = static_cast<unsigned int>(packet.type);
        PacketFunctorBase<T, PT>* pf = new (&functors[type]) PacketFunctor<T, PT, P>(func);
        _packetFunctors[type] = pf;
    }

    bool Execute(T* obj, char* data)
    {
        PacketBase* base = (PacketBase*)data;
        unsigned int type = base->type;

        if (_packetFunctors[type]) {
            return _packetFunctors[type]->Execute(obj, data);
        }
        return false;
    }
};

//
#endif //__PACKET_H__