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

//
bool App::Init()
{
	Network& net = Network::GetInstance();
	net._config.workerThreadCount = 2;
	net._config.doListen = false;

	if (false == net.Initialize()) {
		LogError("Network Initialize fail");
		return false;
	}

	_threads.emplace_back(jthread{ &App::ProcessThread, this, _threadStop.get_token() });
	_threads.emplace_back(jthread{ &App::CommandThread, this, _threadStop.get_token() });
	_threads.emplace_back(jthread{ &App::UpdateThread, this, _threadStop.get_token() });

	return true;
}

bool App::Start()
{
	_threadSuspended = 0;
	_threadSuspended.notify_all();

	Network& net = Network::GetInstance();
	if (false == net.Start()) {
		LogError("Network start fail");
		return false;
	}
	return true;
}

void App::Wait()
{
	_threadWait.wait(1);
}

bool App::Stop()
{
	Network& net = Network::GetInstance();
	if (!net.Stop()) {
		LogError("Network stop fail");
	}
	_threadStop.request_stop();
	CRecvPacketQueue::GetInstance().ForceActivateQueueEvent();

	_threadWait = 0;
	_threadWait.notify_all();

	return true;
}

int App::ProcessThread(stop_token token)
{
	Network& net = Network::GetInstance();
	CRecvPacketQueue& recvPacketQueue = CRecvPacketQueue::GetInstance();
	CUserSessionMgr& sessionMgr = CUserSessionMgr::GetInstance();

	CConnector* pConnector{ nullptr };
	CUserSession* pUserSession{ nullptr };
	CPacketStruct* pPacket{ nullptr };

	DWORD dwPacketSize{ 0 };

	//
	_threadSuspended.wait(1);
	Log(format("{} start", source_location::current().function_name()));
	while (!token.stop_requested()) {
		pPacket = recvPacketQueue.Pop();
		if (!pPacket) {
			continue;
		}

		sPacketHead* pHead = (sPacketHead*)pPacket->_pBuffer;
		sPacketTail* pTail = (sPacketTail*)(pPacket->_pBuffer + pHead->dwLength - sizeof(sPacketTail));
		if (pTail->dwCheckTail != PACKET_CHECK_TAIL_KEY) {
			LogError("invalid packet");
			net.Disconnect(pConnector);
		}

		pConnector = pPacket->pSession;
		if (!pConnector) {
			recvPacketQueue.ReleasePacketStruct(pPacket);
			continue;
		}

		if (pUserSession = (CUserSession*)pConnector->GetParam()) {
			pUserSession->MessageProcess(pPacket->_pBuffer, pPacket->_nDataSize);
		} else {
			sPacketHead* pHeader = (sPacketHead*)pPacket->_pBuffer;
			if (P_AUTH == pHeader->dwProtocol) {
				if (pUserSession = sessionMgr.GetFreeUserSession()) {
					pConnector->SetParam((void*)pUserSession);
					pUserSession->SetConnector(pConnector);
				} else {
					LogError("UserSessionMgr::GetFreeUserSession fail");
				}
			} else {
				LogError("invalid packet");
				net.Disconnect(pConnector);
			}
		}
		 
		recvPacketQueue.ReleasePacketStruct(pPacket);
	}

	Log(format("{} end", source_location::current().function_name()));
	return 0;
}

int App::UpdateThread(stop_token token)
{
	Network& net = Network::GetInstance();
	CUserSessionMgr& sessionMgr = CUserSessionMgr::GetInstance();

	CUserSession* pUerSession{ nullptr };
	INT64 uiCurrTime{ 0 };
	std::list<CUserSession*> sessionList{};

	//
	_threadSuspended.wait(1);
	Log(format("{} start", source_location::current().function_name()));
	while (!token.stop_requested()) {
		sessionMgr.GetUserSessionList(sessionList);

		for (CUserSession* pSession : sessionList) {
			pSession->DoUpdate(uiCurrTime);
		}
	}

	Log(format("{} end", source_location::current().function_name()));
	return 0;
}

int App::CommandThread(stop_token token)
{
	Network& net = Network::GetInstance();

	CConnector* pConnector{ nullptr };
	CUserSession* pSession{ nullptr };

	INT64 biCurrTime{ 0 };
	char cmd[1024]{};

	WCHAR wszHostIP[] = L"127.0.0.1";
	WORD wHostPort = 60010;

	//
	_threadSuspended.wait(1);
	Log(format("{} start", source_location::current().function_name()));
	while (!token.stop_requested()) {
		gets_s(cmd, 1024);

		string cmdStr = cmd;
		if (0 >= cmdStr.length()) {
			continue;
		}

		vector<string> cmdTokens{};
		TokenizeA(cmdStr, cmdTokens, " ");
		if (0 == strncmp(cmdTokens[0].c_str(), "/connect", cmdTokens[0].length())) {
			Log(format(L"info: try connect: {} {}", wszHostIP, wHostPort));
			if (pConnector = net.Connect(wszHostIP, wHostPort)) {
				Log("info: connected");

				if (pSession = CUserSessionMgr::GetInstance().GetFreeUserSession()) {
					pConnector->SetParam(pSession);
					pSession->SetConnector(pConnector);
				} else {
					LogError("CUserSesionMgr::GetFreeUserSession fail");
					net.Disconnect(pConnector);
				}
			}
		} else if (0 == strncmp(cmdTokens[0].c_str(), "/disconnect", cmdTokens[0].length())) {
			if (pConnector) net.Disconnect(pConnector);
		} else if (0 == strncmp(cmdTokens[0].c_str(), "/auth", cmdTokens[0].length())) {
			if (pSession) pSession->ReqAuth();
		} else if (0 == strncmp(cmdTokens[0].c_str(), "/send", cmdTokens[0].length())) {
			if (pSession) pSession->ReqEcho();
		} else if (0 == strncmp(cmdTokens[0].c_str(), "/sendLoop", cmdTokens[0].length())) {
			int loopCount{ 5 };
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
		}
	}

	Log(format("{} end", source_location::current().function_name()));
	return 0;
}