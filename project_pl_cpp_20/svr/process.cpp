#include "stdafx.h"

#include "./process.h"

#include "../_framework/connector_mgr.h"
#include "../_framework/packet_data_queue.h"
#include "../_lib/log.h"

#include "./packet_svr.h"
#include "./user_session_mgr.h"
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
bool App::Init()
{
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
    CRecvPacketQueue::GetInstance().ForceActivateQueueEvent();
    CSendPacketQueue::GetInstance().ForceActivateQueueEvent();

    for (auto& t : _threads) {
        t.join();
    }

    return true;
}

unsigned int App::ProcessThread(stop_token token)
{
    Log(format("log: {} create", "App::ProcessThread"));
    _threadSuspended.wait(1);

    Network& net{Network::GetInstance()};

    WCHAR wcsHostIP[]{L"127.0.0.1"};
    WORD wHostPort{60010};

    net._config.doListen = true;
    wcsncpy_s(net._config.listenInfo.wcsIP, NetworkConst::MAX_LEN_IP4_STRING, wcsHostIP, lstrlenW(wcsHostIP));
    net._config.listenInfo.wPort = wHostPort;

    if (false == net.Initialize()) {
        Log(format("error: App::{}: network initialize fail", source_location::current().function_name()));
        return 1;
    }
    if (false == net.Start()) {
        Log(format("error: App::{}: network start fail", source_location::current().function_name()));
        return 1;
    }

    CRecvPacketQueue& RecvPacketQueue{CRecvPacketQueue::GetInstance()};
    CSendPacketQueue& SendPacketQueue{CSendPacketQueue::GetInstance()};
    UserSessionMgr& userSessionMgr{UserSessionMgr::GetInstance()};

    Connector* pConnector{nullptr};
    UserSession* pUserSession{nullptr};
    CPacketStruct* pPacket{nullptr};

    DWORD dwPacketSize{0};

    //
    Log(format("log: {}: start", "App::ProcessThread"));
    while (!token.stop_requested()) {
        pPacket = RecvPacketQueue.Pop();
        if (!pPacket) {
            continue;
        }

        sPacketHead* pHead = (sPacketHead*)pPacket->_pBuffer;
        sPacketTail* pTail = (sPacketTail*)(pPacket->_pBuffer + pHead->dwLength - sizeof(sPacketTail));
        if (PACKET_CHECK_TAIL_KEY != pTail->dwCheckTail) {
            Log("Invliad packet");
            net.Disconnect(pConnector);
        }

        pConnector = pPacket->pConnector;
        if (!pConnector) {
            RecvPacketQueue.ReleasePacketStruct(pPacket); //SAFE_DELETE(pPacket);
            continue;
        }

        //
        if (pUserSession = (UserSession*)pConnector->GetParam()) {
            //CPerformanceCheck(L"ProcessThread(): UserSession::MessageProcess()");
            pUserSession->MessageProcess(pPacket->_pBuffer, pPacket->_nDataSize);
        } else {
            sPacketHead* pHeader = (sPacketHead*)pPacket->_pBuffer;
            if ((PacketTypeC2S)pHeader->dwProtocol == PacketTypeC2S::auth) {
                //SafeLock lock(userSessionMgr._Lock);

                if (pUserSession = userSessionMgr.GetFreeObject()) {
                    pConnector->SetParam((void*)pUserSession);
                    pUserSession->SetConnector(pConnector);

                    pUserSession->MessageProcess(pPacket->_pBuffer, pPacket->_nDataSize);
                } else {
                    Log("UserSessionMgr::GetFreeUserSession() fail");
                }
            } else {
                Log("Invliad packet");
                net.Disconnect(pConnector);
            }
        }
        RecvPacketQueue.ReleasePacketStruct(pPacket); //SAFE_DELETE(pPacket);

    }

    net.Stop();

    Log(format("log: {}: end", "App::ProcessThread"));
    return 1;
};

//
unsigned int App::UpdateThread(stop_token token)
{
    Log(format("log: {} create", "App::UpdateThread"));
    _threadSuspended.wait(1);

    Network& Net{Network::GetInstance()};
    INT64 biCurrTime{0};
    INT64 biUserSessionMgrUpdate{0};
    INT64 biMonitorReport{0};

    //
    Log(format("log: {}: start", "App::UpdateThread"));
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
                g_PerformanceLog.Write(wstrReport.c_str());
            }
            {
                wstring wstrReport{};
                wstrReport.append(format(L"UserSessionState: {}", UserSessionMgr::GetInstance().GetReport()));
                g_PerformanceLog.Write(wstrReport.c_str());
            }
        }

        //
        _commandQueue.Tick();

        //
        this_thread::sleep_for(1ms);
    }

    Log(format("log: {}: end", "App::UpdateThread"));
    return 1;
}
