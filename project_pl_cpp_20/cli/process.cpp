#include "process.h"

#include "../_framework/connector_mgr.h"
#include "../_framework/packet_data_queue.h"
#include "../_lib/log.h"

#include "./packet_cli.h"
#include "./user_session_mgr.h"
#include "./character.h"
//#include "Config.h"

#include <iostream>
#include <format>
#include <source_location>

// command unit -----------------------------------------------------------------
CommandUnitQueue& GetCmdQueue()
{
    return App::GetInstance().GetCmdQueue();
}

// App --------------------------------------------------------------------------
bool App::Start()
{
    Network& net{Network::GetInstance()};
    net._config.workerThreadCount = 2;
    net._config.doListen = false;

    if (false == net.Initialize()) {
        LogError("Network Initialize fail");
        return false;
    }

    _threads.emplace_back(thread{&App::ProcessThread, this, _threadStop.get_token()});
    _threads.emplace_back(thread{&App::CommandThread, this, _threadStop.get_token()});
    //_threads.emplace_back(thread{&App::UpdateThread, this, _threadStop.get_token()});

    //
    _threadSuspended = 0;
    _threadSuspended.notify_all();

    if (false == net.Start()) {
        LogError("Network start fail");
        return false;
    }

    for (auto& t : _threads) {
        t.join();
    }

    return true;
}

bool App::Stop()
{
    Network& net{Network::GetInstance()};
    if (!net.Stop()) {
        LogError("Network stop fail");
    }

    _threadStop.request_stop();
    CRecvPacketQueue::GetInstance().ForceActivateQueueEvent();

    return true;
}

unsigned int App::ProcessThread(stop_token token)
{
    Log(format("log: {} create", "App::ProcessThread"));
    _threadSuspended.wait(1);

    Network& net{Network::GetInstance()};
    CRecvPacketQueue& recvPacketQueue{CRecvPacketQueue::GetInstance()};
    UserSessionMgr& sessionMgr{UserSessionMgr::GetInstance()};

    Connector* pConnector{nullptr};
    UserSession* pUserSession{nullptr};
    CPacketStruct* pPacket{nullptr};

    DWORD dwPacketSize{0};

    //
    Log(format("log: {}: start", "App::ProcessThread"));
    while (!token.stop_requested()) {
        pPacket = recvPacketQueue.Pop();
        if (!pPacket) {
            continue;
        }

        sPacketHead* pHead = (sPacketHead*)pPacket->_pBuffer;
        sPacketTail* pTail = (sPacketTail*)(pPacket->_pBuffer + pHead->dwLength - sizeof(sPacketTail));
        if (PACKET_CHECK_TAIL_KEY != pTail->dwCheckTail) {
            LogError("invalid packet");
            net.Disconnect(pConnector);
        }

        pConnector = pPacket->pConnector;
        if (!pConnector) {
            recvPacketQueue.ReleasePacketStruct(pPacket);
            continue;
        }

        if (pUserSession = (UserSession*)pConnector->GetParam()) {
            pUserSession->MessageProcess(pPacket->_pBuffer, pPacket->_nDataSize);
        } else {
            sPacketHead* pHeader = (sPacketHead*)pPacket->_pBuffer;
            if ((PacketTypeS2C)pHeader->dwProtocol == PacketTypeS2C::auth_result) {
                //if (pUserSession = sessionMgr.GetFreeObject()) {
                //    pConnector->SetParam((void*)pUserSession);
                //    pUserSession->SetConnector(pConnector);
                //} else {
                //    LogError("UserSessionMgr::GetFreeUserSession fail");
                //}
            } else {
                LogError("invalid packet");
                net.Disconnect(pConnector);
            }
        }

        recvPacketQueue.ReleasePacketStruct(pPacket);

        //
        _commandQueue.Tick();
    }

    Log(format("log: {}: end", "App::ProcessThread"));
    return 0;
}

unsigned int App::UpdateThread(stop_token token)
{
    Log(format("log: {} create", "App::UpdateThread"));
    _threadSuspended.wait(1);

    Network& net{Network::GetInstance()};
    UserSessionMgr& sessionMgr{UserSessionMgr::GetInstance()};

    UserSession* pUerSession{nullptr};
    INT64 uiCurrTime{0};
    list<UserSession*> sessionList{};

    //
    Log(format("log: {}: start", "App::UpdateThread"));
    while (!token.stop_requested()) {
        //sessionMgr.GetUserSessionList(sessionList);

        //for (UserSession* pSession : sessionList) {
        //    pSession->DoUpdate(uiCurrTime);
        //}
    }

    Log(format("log: {}: end", "App::UpdateThread"));
    return 0;
}

unsigned int App::CommandThread(stop_token token)
{
    Log(format("log: {} create", "App::CommandThread"));
    _threadSuspended.wait(1);

    Network& net = Network::GetInstance();

    Connector* pConnector{nullptr};
    UserSession* pSession{nullptr};

    INT64 biCurrTime{0};
    char cmd[1024]{};

    WCHAR wszHostIP[] = L"127.0.0.1";
    WORD wHostPort = 60010;

    //
    Log(format("log: {}: start", "App::CommandThread"));
    while (!token.stop_requested()) {
        gets_s(cmd, 1024);

        string cmdStr = cmd;
        if (0 >= cmdStr.length()) {
            continue;
        }

        vector<string> cmdTokens{};
        TokenizeA(cmdStr, cmdTokens, " ");
        if (0 == strncmp(cmdTokens[0].c_str(), "/connect", cmdTokens[0].length())) {
            if( !pConnector) {
                Log(format(L"info: try connect: {} {}", wszHostIP, wHostPort));
                if (pConnector = net.Connect(wszHostIP, wHostPort)) {
                    Log("info: connected");

                    if (pSession = UserSessionMgr::GetInstance().GetFreeObject()) {
                        pConnector->SetParam(pSession);
                        pSession->SetConnector(pConnector);
                    } else {
                        LogError("CUserSesionMgr::GetFreeUserSession fail");
                        net.Disconnect(pConnector);
                    }
                }
            } else {
                Log("info: already connected");
            }
        } else if (0 == strncmp(cmdTokens[0].c_str(), "/disconnect", cmdTokens[0].length())) {
            if (pConnector) {
                net.Disconnect(pConnector);
                pConnector = nullptr;
            }
        } else if (0 == strncmp(cmdTokens[0].c_str(), "/auth", cmdTokens[0].length())) {
            if (pSession) {
                pSession->ReqAuth();
            }
        } else if (0 == strncmp(cmdTokens[0].c_str(), "/enter", cmdTokens[0].length())) {
            pSession->ReqEnter();
        } else if (0 == strncmp(cmdTokens[0].c_str(), "/leave", cmdTokens[0].length())) {
        } else if (0 == strncmp(cmdTokens[0].c_str(), "/move", cmdTokens[0].length())) {
            auto ch = CharacterMgr::GetInstance().GetPlayerCharacter();
            auto [x, y] = ch->GetPos();
            x += (int)((rand() % 10) * (rand() % 2 == 0 ? 1 : -1));
            y += (int)((rand() % 10) * (rand() % 2 == 0 ? 1 : -1));

            pSession->ReqMove(x, y);
        } else if (0 == strncmp(cmdTokens[0].c_str(), "/send", cmdTokens[0].length())) {
            if (pSession) {
                pSession->ReqEcho();
            }
        } else if (0 == strncmp(cmdTokens[0].c_str(), "/sendLoop", cmdTokens[0].length())) {
            int loopCount{5};
            if (2 <= cmdTokens.size()) {
                loopCount = atoi(cmdTokens[1].c_str());
            }
            if (1 > loopCount) {
                loopCount = 1;
            }

            for (int cnt = 0; cnt < loopCount; ++cnt) {
                if (pSession) {
                    pSession->ReqEcho();
                }
                this_thread::sleep_for(100ms);
            }
        } else if (0 == strncmp(cmdTokens[0].c_str(), "/exit", cmdTokens[0].length())) {
            Stop();
            break;
        } else {
            _commandQueue.Add(new TestCommandUnit);

            DynamicCommandUnit::Operation cmd = []() {
                cout << "call dynamic oerator" << endl;
            };
            _commandQueue.Add(new DynamicCommandUnit(cmd));
        }
    }

    Log(format("log: {}: end", "App::CommandThread"));
    return 0;
}