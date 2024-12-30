#include "process.h"

#include "../_framework/Connector.h"
#include "../_framework/PacketDataQueue.h"
#include "../_framework/Packet.h"
#include "../_framework/log.h"
#include "../_framework/Packet_Protocol.h"

#include "UserSession.h"
//#include "Config.h"

#include <iostream>
#include <format>

WCHAR HOST_ADDRESS[1024 + 1] = L"127.0.0.1";
WORD HOST_PORT = 20001;

void WriteMiniNetLog(std::wstring wstr)
{
	g_Log.Write(wstr);
}

App::App()
{
}

App::~App()
{
}

bool App::Init()
{
	unsigned int uiThreadID = 0;

	{
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, UpdateThread, this, CREATE_SUSPENDED, &uiThreadID);
		if (NULL == hThread) {
			WriteMiniNetLog(L"error: App::Init(): UpdateThread create fail");
			return false;
		}

		_threadHandleSet.insert(hThread);
	}
	{
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, ProcessThread, NULL, CREATE_SUSPENDED, &uiThreadID);
		if (NULL == hThread) {
			WriteMiniNetLog(L"error: App::Init(): ProcessThread create fail");
			return false;
		}

		_threadHandleSet.insert(hThread);
	}
	{
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, MonitorThread, NULL, CREATE_SUSPENDED, &uiThreadID);
		if (NULL == hThread) {
			WriteMiniNetLog(L"error: App::Init(): MonitorThread create fail");
			return false;
		}

		_threadHandleSet.insert(hThread);
	}

	return true;
}

void App::Run()
{
	InterlockedExchange((LONG*)&_dwRunning, 1);

	for (HANDLE hThread : _threadHandleSet) {
		ResumeThread(hThread);
	}
}

void App::Stop()
{
	InterlockedExchange((LONG*)&_dwRunning, 0);

	for (HANDLE hThread : _threadHandleSet) {
		WaitForSingleObject(hThread, INFINITE);
	}
}

unsigned int WINAPI App::ProcessThread(void *p)
{
	Network &net = Network::GetInstance();

	WCHAR wcsHostIP[] = L"127.0.0.1";
	WORD wHostPort = 20010;

	net._config.doListen = true;
	wcsncpy_s(net._config.listenInfo.wcsIP, eNetwork::MAX_LEN_IP4_STRING, wcsHostIP, lstrlenW(wcsHostIP));
	net._config.listenInfo.wPort = wHostPort;

	if (false == net.Initialize()) {
		WriteMiniNetLog(L"error: App::ProcessThread: network initialize fail");
		return 1;
	}
	if (false == net.Start()) {
		WriteMiniNetLog(L"error: App::ProcessThread: network start fail");
		return 1;
	}

	CRecvPacketQueue &RecvPacketQueue = CRecvPacketQueue::GetInstance();
	CSendPacketQueue &SendPacketQueue = CSendPacketQueue::GetInstance();
	CUserSessionMgr &UserSessionMgr = CUserSessionMgr::GetInstance();

	//
	DWORD &dwRunning = net._dwRunning;

	CConnector *pConnector = nullptr;
	CUserSession *pUserSession = nullptr;
	CPacketStruct *pPacket = nullptr;

	DWORD dwPacketSize = 0;

	//
	while( 1 == InterlockedExchange(&dwRunning, dwRunning) )
	{
		pPacket = RecvPacketQueue.Pop();
		if( !pPacket )
			continue;

		sPacketHead *pHead = (sPacketHead*)pPacket->_pBuffer;
		sPacketTail *pTail = (sPacketTail*)(pPacket->_pBuffer + pHead->dwLength - sizeof(sPacketTail));
		if( pTail->dwCheckTail != PACKET_CHECK_TAIL_KEY )
		{
			WriteMiniNetLog(L"Invliad packet");
			net.Disconnect(pConnector);
		}

		pConnector = pPacket->pSession;
		if( !pConnector )
		{
			RecvPacketQueue.ReleasePacketStruct(pPacket); //SAFE_DELETE(pPacket);
			continue;
		}

		//
		pUserSession = (CUserSession*)pConnector->GetParam();
		if( nullptr == pUserSession )
		{
			sPacketHead *pHeader = (sPacketHead*)pPacket->_pBuffer;
			if( P_AUTH == pHeader->dwProtocol )
			{
				SafeLock lock(UserSessionMgr._Lock);
				pUserSession = UserSessionMgr.GetFreeUserSession();
				if( !pUserSession )
				{
					WriteMiniNetLog(L"CUserSessionMgr::GetFreeUserSession() fail");
				}
				else
				{
					pConnector->SetParam((void*)pUserSession);
					pUserSession->SetConnector(pConnector);
				}
			}
			else
			{
				WriteMiniNetLog(L"Invliad packet");
				net.Disconnect(pConnector);
			}
		}
		else
		{
			//CPerformanceCheck(L"ProcessThread(): UserSession::MessageProcess()");
			pUserSession->MessageProcess(pPacket->_pBuffer, pPacket->_nDataSize);
		}

		//
		RecvPacketQueue.ReleasePacketStruct(pPacket); //SAFE_DELETE(pPacket);
	}

	net.Stop();

	//
	return 0;
};

//
unsigned int WINAPI App::UpdateThread(void *p)
{
	Network &Net = Network::GetInstance();
	//CUserSessionMgr &UserSessionMgr = CUserSessionMgr::GetInstance();

	App *app = (App*)p;

	//
	DWORD &dwRunning = Net._dwRunning;
	INT64 biCurrTime = 0;

	CUserSession *pUserSession = nullptr;
	//std::list<CUserSession*> ReleaseSessionList = {};

	while( 1 == InterlockedExchange(&dwRunning, dwRunning) )
	{
		biCurrTime = GetTimeMilliSec();

		//
		app->SessionRelease(biCurrTime);

		//
		Sleep(1);
	}

	//
	return 0;
}

bool App::SessionRelease(INT64 biCurrTime)
{
	CUserSessionMgr& UserSessionMgr = CUserSessionMgr::GetInstance();
	std::list<CUserSession*> ReleaseSessionList = {};

	{
		SafeLock lock(UserSessionMgr._Lock);
		for( CUserSession *pSession : UserSessionMgr._UsedUserSessionList )
		{
			if( pSession->CheckUpdateTime(biCurrTime) )
				continue;

			pSession->SetUpdateTime(biCurrTime + MILLISEC_A_SEC);

			//
			CConnector *pConnector = pSession->GetConnector();
			if( !pConnector || !pConnector->GetActive() )
			{
				ReleaseSessionList.push_back(pSession);
				continue;
			}

			pSession->DoUpdate(biCurrTime);
		}
	}

	{
		for( CUserSession *pSession : ReleaseSessionList )
		{
			pSession->Release();
			UserSessionMgr.ReleaseUserSesssion(pSession);
		}

		ReleaseSessionList.clear();
	}

	return true;
}

unsigned int WINAPI App::MonitorThread(void *p)
{
	Network &Net = Network::GetInstance();
	CUserSessionMgr &UserSessionMgr = CUserSessionMgr::GetInstance();

	//
	DWORD &dwRunning = Net._dwRunning;
	INT64 biCurrTime = 0;
	INT64 biCheckTimer = GetTimeMilliSec() + (MILLISEC_A_SEC * 15);

	//
	while( 1 == InterlockedExchange(&dwRunning, dwRunning) )
	{
		biCurrTime = GetTimeMilliSec();

		//
		if( biCheckTimer < biCurrTime )
		{
			CConnector *pConnector = Net.Connect(HOST_ADDRESS, HOST_PORT);
			if( !pConnector )
			{
				WriteMiniNetLog(L"error: MonitorThread: Connect fail");
			}
			else
			{
				g_Log.Write(L"log: MonitorThread: <%d> Connected. Socket %d", pConnector->GetIndex(), pConnector->GetSocket());
				//WriteMiniNetLog(format(L"log: MonitorThread: <{}> Connected. Socket {}", pConnector->GetIndex(), pConnector->GetSocket()));
				Net.Disconnect(pConnector);
			}

			biCheckTimer = GetTimeMilliSec() + (MILLISEC_A_SEC * 15);
		}

		//
		Sleep(1);
	}

	//
	return 0;
}