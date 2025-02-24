#include "process.h"

#include "../_framework/Connector.h"
#include "../_framework/PacketDataQueue.h"
#include "../_framework/Packet.h"
#include "../_framework/Packet_Protocol.h"
#include "../_lib/log.h"

#include "UserSession.h"
//#include "Config.h"

#include <iostream>
#include <format>
#include <source_location>
#include <thread>
#include <mutex>

WCHAR HOST_ADDRESS[1024 + 1] = L"127.0.0.1";
WORD HOST_PORT = 20001;

App::App()
{
}

App::~App()
{
}

bool App::Init()
{
    _threads.emplace_back(&App::UpdateThread, this, _threadStop.get_token());
    _threads.emplace_back(&App::ProcessThread, this, _threadStop.get_token());
    _threads.emplace_back(&App::MonitorThread, this, _threadStop.get_token());

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
    Log(format("log: {} create", source_location::current().function_name()));
    _threadSuspended.wait(1);

    Network& net{Network::GetInstance()};

    WCHAR wcsHostIP[]{L"127.0.0.1"};
    WORD wHostPort{60010};

    net._config.doListen = true;
    wcsncpy_s(net._config.listenInfo.wcsIP, eNetwork::MAX_LEN_IP4_STRING, wcsHostIP, lstrlenW(wcsHostIP));
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
    CUserSessionMgr& UserSessionMgr{CUserSessionMgr::GetInstance()};

    //
    CConnector* pConnector{nullptr};
    CUserSession* pUserSession{nullptr};
    CPacketStruct* pPacket{nullptr};

    DWORD dwPacketSize{0};

    //
    Log(format("log: {}: start", source_location::current().function_name()));
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

        pConnector = pPacket->pSession;
        if (!pConnector) {
            RecvPacketQueue.ReleasePacketStruct(pPacket); //SAFE_DELETE(pPacket);
            continue;
        }

        //
        pUserSession = (CUserSession*)pConnector->GetParam();
        if (nullptr == pUserSession) {
            sPacketHead* pHeader = (sPacketHead*)pPacket->_pBuffer;
            if (P_AUTH == pHeader->dwProtocol) {
                SafeLock lock(UserSessionMgr._Lock);
                pUserSession = UserSessionMgr.GetFreeUserSession();
                if (!pUserSession) {
                    Log("CUserSessionMgr::GetFreeUserSession() fail");
                } else {
                    pConnector->SetParam((void*)pUserSession);
                    pUserSession->SetConnector(pConnector);
                }
            } else {
                Log("Invliad packet");
                net.Disconnect(pConnector);
            }
        } else {
            //CPerformanceCheck(L"ProcessThread(): UserSession::MessageProcess()");
            pUserSession->MessageProcess(pPacket->_pBuffer, pPacket->_nDataSize);
        }

        //
        RecvPacketQueue.ReleasePacketStruct(pPacket); //SAFE_DELETE(pPacket);
    }

    net.Stop();

    Log(format("log: {}: end", source_location::current().function_name()));
    return 1;
};

//
unsigned int App::UpdateThread(stop_token token)
{
    Log(format("log: {} create", source_location::current().function_name()));
    _threadSuspended.wait(1);

    Network& Net{Network::GetInstance()};
    //CUserSessionMgr &UserSessionMgr{CUserSessionMgr::GetInstance()};

    //
    INT64 biCurrTime{0};

    CUserSession* pUserSession{nullptr};
    //std::list<CUserSession*> ReleaseSessionList{};

    //
    Log(format("log: {}: start", source_location::current().function_name()));
    while (!token.stop_requested()) {
        biCurrTime = GetTimeMilliSec();

        //
        SessionRelease(biCurrTime);

        //
        this_thread::sleep_for(1ms);
    }

    Log(format("log: {}: end", source_location::current().function_name()));
    return 1;
}

unsigned int App::MonitorThread(stop_token token)
{
    Log(format("log: {} create", source_location::current().function_name()));
    _threadSuspended.wait(1);

    Network& Net{Network::GetInstance()};
    CUserSessionMgr& UserSessionMgr{CUserSessionMgr::GetInstance()};

    //
    INT64 biCurrTime{0};
    INT64 biCheckTimer{GetTimeMilliSec() + (MILLISEC_A_SEC * 15)};

    //
    Log(format("log: {}: start", source_location::current().function_name()));
    while (!token.stop_requested()) {
        //biCurrTime = GetTimeMilliSec();

        ////
        //if( biCheckTimer < biCurrTime )
        //{
        //	CConnector *pConnector = Net.Connect(HOST_ADDRESS, HOST_PORT);
        //	if( !pConnector )
        //	{
        //		WriteMiniNetLog(L"error: MonitorThread: Connect fail");
        //	}
        //	else
        //	{
        //		g_Log.Write(L"log: MonitorThread: <%d> Connected. Socket %d", pConnector->GetIndex(), pConnector->GetSocket());
        //		//WriteMiniNetLog(format(L"log: MonitorThread: <{}> Connected. Socket {}", pConnector->GetIndex(), pConnector->GetSocket()));
        //		Net.Disconnect(pConnector);
        //	}

        //	biCheckTimer = GetTimeMilliSec() + (MILLISEC_A_SEC * 15);
        //}

        //
        //Sleep(1);
        this_thread::sleep_for(1ms);
    }

    Log(format("log: {}: end", source_location::current().function_name()));
    return 1;
}

bool App::SessionRelease(INT64 biCurrTime)
{
    CUserSessionMgr& UserSessionMgr{CUserSessionMgr::GetInstance()};
    std::list<CUserSession*> ReleaseSessionList{};

    {
        SafeLock lock(UserSessionMgr._Lock);
        for (CUserSession* pSession : UserSessionMgr._UsedUserSessionList) {
            if (pSession->CheckUpdateTime(biCurrTime)) {
                continue;
            }

            pSession->SetUpdateTime(biCurrTime + MILLISEC_A_SEC);

            //
            CConnector* pConnector = pSession->GetConnector();
            if (!pConnector || !pConnector->GetActive()) {
                ReleaseSessionList.push_back(pSession);
                continue;
            }

            pSession->DoUpdate(biCurrTime);
        }
    }

    {
        for (CUserSession* pSession : ReleaseSessionList) {
            pSession->Release();
            UserSessionMgr.ReleaseUserSesssion(pSession);
        }

        ReleaseSessionList.clear();
    }

    return true;
}
