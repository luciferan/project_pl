#include "../_framework/connector.h"
#include "../_framework/packet.h"
#include "../_lib/util.h"

#include "./user_session.h"

#include <iostream>

using namespace std;

//
int CUserSession::clear()
{
    InterlockedExchange(&_dwUsed, 0);
    InterlockedExchange(&_dwActive, 0);

    _pConnector->SetParam(nullptr);

    return 0;
}

int CUserSession::MessageProcess(char* pData, int nLen)
{
    if (sizeof(sPacketHead) > nLen) {
        return -1;
    }

    sPacketHead* pHeader = (sPacketHead*)pData;
    DWORD dwPackethLength = pHeader->dwLength - sizeof(sPacketHead) - sizeof(sPacketTail);
    DWORD dwProtocol = pHeader->dwProtocol;

    char* pPacketData = (char*)(pHeader + 1);

    PacketTypeS2C packetType = (PacketTypeS2C)dwProtocol;
    switch (packetType) {
    case PacketTypeS2C::auth_result:
        S2C_AuthResult(this, *((SC_P_AUTH_RESULT*)pPacketData));
        break;
    case PacketTypeS2C::enter:
        S2C_Enter(this, *((SC_P_ENTER*)pPacketData));
        break;
    case PacketTypeS2C::move:
        S2C_Move(this, *((SC_P_MOVE*)pPacketData));
        break;
    case PacketTypeS2C::interaction:
        S2C_Interaction(this, *((SC_P_INTERACTION*)pPacketData));
        break;
    case PacketTypeS2C::heartbeat:
        S2C_Heartbeat(this, *((SC_P_HEARTBEAT*)pPacketData));
        break;
    case PacketTypeS2C::echo:
        S2C_Echo(this, *((SC_P_ECHO*)pPacketData));
        break;
    default:
        LogError(format("invalid packet protocol {}", dwProtocol));
        break;
    }

    //
    return 0;
}

int CUserSession::DoUpdate(INT64 uiCurrTime)
{
    return 0;
}

eResultCode CUserSession::SendPacketData(DWORD dwProtocol, char* pData, DWORD dwSendDataSize)
{
    //CNetworkBuffer SendBuffer;
    char SendBuffer[MAX_PACKET_BUFFER_SIZE]{};
    DWORD dwSendBufferSize = sizeof(SendBuffer);
    MakeNetworkPacket(dwProtocol, pData, dwSendDataSize, (char*)&SendBuffer, dwSendBufferSize);

    return _pConnector->AddSendData((char*)&SendBuffer, dwSendBufferSize);
}

eResultCode CUserSession::SendPacket(PacketBaseC2S* packetData, DWORD packetSize)
{
    char sendBuffer[MAX_PACKET_BUFFER_SIZE]{};
    DWORD sendBufferSize = MAX_PACKET_BUFFER_SIZE;
    MakeNetworkPacket((DWORD)packetData->type, (char*)packetData, packetSize, sendBuffer, sendBufferSize);

    return _pConnector->AddSendData(sendBuffer, sendBufferSize);
}

//
eResultCode CUserSession::ReqAuth()
{
    int id{(int)(rand() % 10)};

    CS_P_AUTH sendPacket;
    sendPacket.SetId(id);
    return SendPacket(&sendPacket, sizeof(sendPacket));
}

eResultCode CUserSession::ReqEnter()
{
    CS_P_ENTER sendPacket;
    sendPacket.SetToken(_nub.token);
    return SendPacket(&sendPacket, sizeof(sendPacket));
}

eResultCode CUserSession::ReqMove(int posX, int posY)
{
    CS_P_MOVE sendPacket;
    sendPacket.SetPos(posX, posY);
    return SendPacket(&sendPacket, sizeof(sendPacket));
}

eResultCode CUserSession::ReqInteraction(int targetId, int type)
{
    CS_P_INTERACTION sendPacket;
    sendPacket.SetInteraction(targetId, type);
    return SendPacket(&sendPacket, sizeof(sendPacket));
}

eResultCode CUserSession::ReqEcho()
{
    string echoMsg{format("hello. my id {}.",_nub.token)};
    CS_P_ECHO sendPacket;
    sendPacket.SetData(echoMsg.c_str(), echoMsg.length());
    return SendPacket(&sendPacket, sizeof(sendPacket));
}

eResultCode CUserSession::RepHeartBeat()
{
    CS_P_HEARTBEAT sendPacket;
    return SendPacket(&sendPacket, sizeof(sendPacket));
}

bool S2C_AuthResult(CUserSession* userSession, const SC_P_AUTH_RESULT& packet)
{
    userSession->_nub.id = packet.id;
    userSession->_nub.token = packet.token;
    return true;
}

bool S2C_Enter(CUserSession* userSession, const SC_P_ENTER& packet)
{
    if (packet.token == userSession->_nub.token) {
        userSession->_nub.pos_x = packet.x;
        userSession->_nub.pos_y = packet.y;
        cout << format("입장. 현재위치: {}, {}", userSession->_nub.pos_x, userSession->_nub.pos_y) << endl;
    } else {
        cout << format("{} 입장. 위치: {}, {}", packet.token, packet.x, packet.y) << endl;
    }
    return true;
}

bool S2C_Move(CUserSession* userSession, const SC_P_MOVE& packet)
{
    if (packet.token == userSession->_nub.token) {
        userSession->_nub.pos_x = packet.x;
        userSession->_nub.pos_y = packet.y;
        cout << format("이동: {}, {}", userSession->_nub.pos_x, userSession->_nub.pos_y) << endl;
    } else {
        cout << format("{}가 이동: {}, {}", packet.token, packet.x, packet.y) << endl;

    }
    return true;
}

bool S2C_Interaction(CUserSession* userSession, const SC_P_INTERACTION& packet)
{
    if( packet.token == userSession->_nub.token ){
        cout << "당신은 " << packet.targetToken << "에게 {" << packet.type << "} 행동을 합니다." << endl;
    } else if( packet.targetToken == userSession->_nub.token) {
        cout << packet.targetToken << "이 당신에게 {" << packet.type << "} 행동을 합니다." << endl;
    } else {
        cout << packet.targetToken << "가 " << packet.targetToken << "에게 {" << packet.type << "} 행동을 합니다." << endl;
    }

    return true;
}

bool S2C_Heartbeat(CUserSession* userSession, const SC_P_HEARTBEAT& packet)
{
    cout << "heartbeat" << endl;

    CS_P_HEARTBEAT sendPacket;
    userSession->SendPacket(&sendPacket, sizeof(sendPacket));
    return true;
}

#include <iostream>
bool S2C_Echo(CUserSession* userSession, const SC_P_ECHO& packet)
{
    cout << "에코: " << packet.echoData << endl;
    return true;
}