#include "stdafx.h"

#include "../_framework/connector_mgr.h"
#include "../_framework/packet_data_queue.h"
#include "../_lib/log.h"

#include "./process.h"
#include "./packet_svr.h"
#include "./user_session_mgr.h"
#include "./config.h"

#include <iostream>
#include <format>
#include <source_location>

// command unit -----------------------------------------------------------------
CommandUnitQueue& GetCmdQueue()
{
    return App::GetInstance().GetCmdQueue();
}

// App --------------------------------------------------------------------------
bool App::Init(ConfigLoader& configLoader)
{
    //WCHAR wcsHostIP[]{L"127.0.0.1"};
    //WORD wHostPort{60010};

    //net._config.doListen = true;
    //wcsncpy_s(net._config.listenClient.wcsIP, NetworkConst::MAX_LEN_IP4_STRING, wcsHostIP, lstrlenW(wcsHostIP));
    //net._config.listenClient.wPort = wHostPort;

    if (false == Network::GetInstance().Initialize(configLoader)) {
        LogError("network initialize fail");
        return false;
    }

    _threads.emplace_back(&App::ProcessThread, this, _threadStop.get_token());
    _threads.emplace_back(&App::UpdateThread, this, _threadStop.get_token());

    return true;
}

bool App::Start()
{
    _threadSuspended = 0;
    _threadSuspended.notify_all();

    return true;
}

bool App::Stop()
{
    _threadStop.request_stop();

    Network::GetInstance().Stop();
    RecvPacketQueue::GetInstance().ForceActivateQueueEvent();
    SendPacketQueue::GetInstance().ForceActivateQueueEvent();

    for (auto& t : _threads) {
        t.join();
    }

    return true;
}

unsigned int App::ProcessThread(stop_token token)
{
    Log(format("log: {}: create", "App::ProcessThread"));
    _threadSuspended.wait(1);

    Network& net{Network::GetInstance()};

    if (false == net.Start()) {
        LogError(format("App::{}: network start fail", source_location::current().function_name()));
        return 1;
    }

    RecvPacketQueue& RecvPacketQueue{RecvPacketQueue::GetInstance()};
    SendPacketQueue& SendPacketQueue{SendPacketQueue::GetInstance()};
    UserSessionMgr& userSessionMgr{UserSessionMgr::GetInstance()};

    Connector* pConnector{nullptr};
    UserSession* pUserSession{nullptr};
    PacketStruct* pPacket{nullptr};

    DWORD dwPacketSize{0};

    //
    Log(format("log: {} start", "App::ProcessThread"));
    while (!token.stop_requested()) {
        pPacket = RecvPacketQueue.Pop();
        if (!pPacket) {
            continue;
        }

        PacketHead* pHead = (PacketHead*)pPacket->_pBuffer;
        PacketTail* pTail = (PacketTail*)(pPacket->_pBuffer + pHead->dwLength - sizeof(PacketTail));
        if (PACKET_CHECK_TAIL_KEY != pTail->dwCheckTail) {
            LogError("Invliad packet");
            net.Disconnect(pConnector);
        }

        pConnector = pPacket->_pConnector;
        if (!pConnector) {
            RecvPacketQueue.ReleasePacketStruct(pPacket); //SAFE_DELETE(pPacket);
            continue;
        }

        //
        if (pUserSession = (UserSession*)pConnector->GetParam()) {
            //CPerformanceCheck(L"ProcessThread(): UserSession::MessageProcess()");
            pUserSession->MessageProcess(pPacket->_pBuffer, pPacket->_nDataSize);
        } else {
            PacketHead* pHeader = (PacketHead*)pPacket->_pBuffer;
            if ((PacketTypeC2S)pHeader->dwProtocol == PacketTypeC2S::auth) {
                //SafeLock lock(userSessionMgr._Lock);

                if (pUserSession = userSessionMgr.GetFreeObject()) {
                    pConnector->SetParam((void*)pUserSession);
                    pUserSession->SetConnector(pConnector);

                    pUserSession->MessageProcess(pPacket->_pBuffer, pPacket->_nDataSize);
                } else {
                    LogError("UserSessionMgr::GetFreeUserSession() fail");
                }
            } else {
                LogError("Invliad packet");
                net.Disconnect(pConnector);
            }
        }
        RecvPacketQueue.ReleasePacketStruct(pPacket); //SAFE_DELETE(pPacket);

    }

    net.Stop();

    Log(format("log: {} end", "App::ProcessThread"));
    return 1;
};

//
unsigned int App::UpdateThread(stop_token token)
{
    Log(format("log: {}: create", "App::UpdateThread"));
    _threadSuspended.wait(1);

    Network& Net{Network::GetInstance()};
    INT64 biCurrTime{0};
    INT64 biUserSessionMgrUpdate{0};
    INT64 biMonitorReport{0};

    //
    Log(format("log: {} start", "App::UpdateThread"));
    while (!token.stop_requested()) {
        biCurrTime = GetTimeMilliSec();

        //
        if (biUserSessionMgrUpdate < biCurrTime) {
            biUserSessionMgrUpdate = biCurrTime + (MILLISEC_A_SEC);
            UserSessionMgr::GetInstance().DoUpdate(biCurrTime);
        }

        //
        if (biMonitorReport < biCurrTime) {
            biMonitorReport = biCurrTime + (MILLISEC_A_SEC * 30);

            {
                wstring wstrReport{};
                wstrReport.append(format(L"ConnectorState: {}", ConnectorMgr::GetInstance().GetReport()));
                PerformanceLog(wstrReport);
            }
            {
                wstring wstrReport{};
                wstrReport.append(format(L"UserSessionState: {}", UserSessionMgr::GetInstance().GetReport()));
                PerformanceLog(wstrReport);
            }
        }

        //
        _commandQueue.Tick();

        //
        this_thread::sleep_for(1ms);
    }

    Log(format("log: {} end", "App::UpdateThread"));
    return 1;
}
