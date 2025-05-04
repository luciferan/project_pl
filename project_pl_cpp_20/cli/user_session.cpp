#include "../_framework/connector.h"
#include "../_framework/packet.h"
#include "../_lib/util.h"

#include "./user_session.h"
#include "./character.h"

#include <iostream>

using namespace std;

//
UserSession::HandlerImpl UserSession::_impl;

int UserSession::clear()
{
    _pConnector->SetParam(nullptr);

    return 0;
}

eResultCode UserSession::SendPacketData(DWORD dwProtocol, char* pData, DWORD dwSendDataSize)
{
    //NetworkBuffer SendBuffer;
    char SendBuffer[MAX_PACKET_BUFFER_SIZE]{};
    DWORD dwSendBufferSize = sizeof(SendBuffer);
    MakeNetworkPacket(dwProtocol, pData, dwSendDataSize, (char*)&SendBuffer, dwSendBufferSize);

    return _pConnector->AddSendData((char*)&SendBuffer, dwSendBufferSize);
}

eResultCode UserSession::SendPacket(PacketBaseC2S* packetData, DWORD packetSize)
{
    char sendBuffer[MAX_PACKET_BUFFER_SIZE]{};
    DWORD sendBufferSize = MAX_PACKET_BUFFER_SIZE;
    MakeNetworkPacket((DWORD)packetData->type, (char*)packetData, packetSize, sendBuffer, sendBufferSize);

    return _pConnector->AddSendData(sendBuffer, sendBufferSize);
}

//
eResultCode UserSession::ReqAuth()
{
    int id{(int)(rand() % 10)};

    CS_P_AUTH sendPacket;
    sendPacket.SetId(id);
    return SendPacket(&sendPacket, sizeof(sendPacket));
}

eResultCode UserSession::ReqEnter()
{
    CS_P_ENTER sendPacket;
    sendPacket.SetToken(GetToken());
    return SendPacket(&sendPacket, sizeof(sendPacket));
}

eResultCode UserSession::ReqMove(int posX, int posY)
{
    CS_P_MOVE sendPacket;
    sendPacket.SetPos(posX, posY);
    return SendPacket(&sendPacket, sizeof(sendPacket));
}

eResultCode UserSession::ReqInteraction(int targetId, int type)
{
    CS_P_INTERACTION sendPacket;
    sendPacket.SetInteraction(targetId, type);
    return SendPacket(&sendPacket, sizeof(sendPacket));
}

eResultCode UserSession::ReqEcho()
{
    string echoMsg{format("hello. my id {}.",GetToken())};
    CS_P_ECHO sendPacket;
    sendPacket.SetData(echoMsg.c_str(), (int)echoMsg.length());
    return SendPacket(&sendPacket, sizeof(sendPacket));
}

eResultCode UserSession::RepHeartBeat()
{
    CS_P_HEARTBEAT sendPacket;
    return SendPacket(&sendPacket, sizeof(sendPacket));
}

int UserSession::DoUpdate(INT64 uiCurrTime)
{
    return 0;
}

int UserSession::MessageProcess(char* pData, int nLen)
{
    if (sizeof(PacketHead) > nLen) {
        return -1;
    }

    PacketHead* pHeader = (PacketHead*)pData;
    DWORD dwPackethLength = pHeader->dwLength - sizeof(PacketHead) - sizeof(PacketTail);
    DWORD dwProtocol = pHeader->dwProtocol;
    char* pPacketData = (char*)(pHeader + 1);

    return _impl.Execute(this, pPacketData);
}

bool S2C_AuthResult(UserSession* userSession, const SC_P_AUTH_RESULT& packet)
{
    userSession->SetToken(packet.token);

    auto character = CharacterMgr::GetInstance().GetPlayerCharacter();
    character->SetId(packet.id);
    character->SetToken(packet.token);

    return true;
}

bool S2C_Enter(UserSession* userSession, const SC_P_ENTER& packet)
{
    INT64 token = packet.token;

    Character* character = nullptr;
    if (CharacterMgr::GetInstance().IsPlayerCharacter(token)) {
        character = CharacterMgr::GetInstance().GetPlayerCharacter();
    } else {
        character = CharacterMgr::GetInstance().EnterCharacter(token);
    }

    if (!character) {
        LogError("invalid character");
        return true;
    }

    character->SetPos(packet.x, packet.y);
    auto [x, y] = character->GetPos();

    if( CharacterMgr::GetInstance().IsPlayerCharacter(character) ){
        cout << format("입장. 현재위치: {}:{}", x, y) << endl;
    } else {
        cout << format("{} 입장. 위치: {}:{}", token, x, y) << endl;
    }

    return true;
}

bool S2C_EnterCharacter(UserSession* userSession, const SC_P_ENTER_CHARACTER_LIST& packet)
{
    for (int idx = 0; idx < packet.count; ++idx) {
        const CharacterDbData& info = packet.data[idx];

        Character* ch = CharacterMgr::GetInstance().EnterCharacter(info._token);
        ch->SetPos(info._pos);

        cout << format("{} 확인. 위치: {}:{}", info._token, info._pos.posX, info._pos.posY) << endl;
    }

    return true;
}

bool S2C_Leave(UserSession* userSession, const SC_P_LEAVE& packet)
{
    INT64 token = packet.token;

    auto character = CharacterMgr::GetInstance().GetCharacter(token);
    if (!character) {
        LogError("invalid character");
        return true;
    }

    if (CharacterMgr::GetInstance().IsPlayerCharacter(character)) {
        cout << "나 퇴장" << endl;
    } else {
        CharacterMgr::GetInstance().LeaveCharacter(token);
        cout << format("{} 퇴장", token) << endl;
    }

    return true;
}

bool S2C_Move(UserSession* userSession, const SC_P_MOVE& packet)
{
    INT64 token = packet.token;

    auto character = CharacterMgr::GetInstance().GetCharacter(token);
    if (!character) {
        LogError("invalid character");
        return true;
    }

    auto [old_x, old_y] = character->GetPos();
    character->SetPos(packet.x, packet.y);
    auto [new_x, new_y] = character->GetPos();

    if (CharacterMgr::GetInstance().IsPlayerCharacter(character)) {
        cout << format("이동: {}:{} -> {}:{}", old_x, old_y, new_x, new_y) << endl;
    } else {
        cout << format("{}가 이동: {}:{} -> {}:{}", packet.token, old_x, old_y, new_x, new_y) << endl;
    }

    return true;
}

bool S2C_Interaction(UserSession* userSession, const SC_P_INTERACTION& packet)
{
    INT64 token = packet.token;
    INT64 targetToken = packet.targetToken;

    auto ch = CharacterMgr::GetInstance().GetCharacter(token);
    if (!ch) {
        LogError("invalid character");
        return true;
    }
    auto targetCh = CharacterMgr::GetInstance().GetCharacter(targetToken);
    if (targetToken != 0 && !targetCh) {
        LogError("invalid target character");
        return true;
    }

    if (CharacterMgr::GetInstance().IsPlayerCharacter(ch)) {
        cout << "당신은 " << targetToken << "에게 {" << packet.type << "} 행동." << endl;
    } else if( CharacterMgr::GetInstance().IsPlayerCharacter(targetCh)) {
        cout << targetToken << "이 당신에게 {" << packet.type << "} 행동." << endl;
    } else {
        cout << token << "가 " << targetToken << "에게 {" << packet.type << "} 행동." << endl;
    }

    return true;
}

bool S2C_Heartbeat(UserSession* userSession, const SC_P_HEARTBEAT& packet)
{
    cout << "heartbeat" << endl;

    CS_P_HEARTBEAT sendPacket;
    userSession->SendPacket(&sendPacket, sizeof(sendPacket));
    return true;
}

bool S2C_Echo(UserSession* userSession, const SC_P_ECHO& packet)
{
    cout << packet.token <<  ": " << packet.echoData << endl;
    return true;
}

void RegisterPacketHandlers(UserSession::HandlerImpl& impl)
{
    impl.Register(&S2C_AuthResult);
    impl.Register(&S2C_Enter);
    impl.Register(&S2C_EnterCharacter);
    impl.Register(&S2C_Leave);
    impl.Register(&S2C_Move);
    impl.Register(&S2C_Interaction);
    impl.Register(&S2C_Heartbeat);
    impl.Register(&S2C_Echo);
}
