#include "process.h"

#include "../_framework/Connector.h"
#include "../_framework/PacketDataQueue.h"
#include "../_framework/Packet.h"
#include "../_framework/Packet_Protocol.h"

#include "UserSession.h"
//#include "Config.h"

#include <iostream>

//
WCHAR HOST_ADDRESS[1024+1] = L"127.0.0.1";
WORD HOST_PORT = 20010;

//
void WriteMiniNetLog(std::wstring wstr)
{
	//return;

	g_Log.Write(wstr);
}

//bool LoadConfig()
//{
//	g_Config.LoadConfig();
//	return true;
//}

unsigned int WINAPI ProcessThread(void *p)
{
	Network &Net = Network::GetInstance();
	CRecvPacketQueue &RecvPacketQueue = CRecvPacketQueue::GetInstance();
	CSendPacketQueue &SendPacketQueue = CSendPacketQueue::GetInstance();
	CUserSessionMgr &UserSessionMgr = CUserSessionMgr::GetInstance();

	//
	DWORD &dwRunning = Net._dwRunning;

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
			g_Log.Write(L"Invliad packet");
			Net.Disconnect(pConnector);
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
			//if( 1 )
			{
				pUserSession = UserSessionMgr.GetFreeUserSession();
				if( !pUserSession )
				{
					g_Log.Write(L"CUserSessionMgr::GetFreeUserSession() fail");
				}
				else
				{
					pConnector->SetParam((void*)pUserSession);
					pUserSession->SetConnector(pConnector);
				}
			}
			else
			{
				g_Log.Write(L"Invliad packet");
				Net.Disconnect(pConnector);
			}
		}
		else
		{
			pUserSession->MessageProcess(pPacket->_pBuffer, pPacket->_nDataSize);
		}

		//
		RecvPacketQueue.ReleasePacketStruct(pPacket); //SAFE_DELETE(pPacket);
	}

	//
	return 0;
};

//
unsigned int WINAPI UpdateThread(void *p)
{
	Network &Net = Network::GetInstance();
	CUserSessionMgr &UserSessionMgr = CUserSessionMgr::GetInstance();

	//
	DWORD &dwRunning = Net._dwRunning;
	INT64 uiCurrTime = 0;

	CUserSession *pUserSession = nullptr;

	while( 1 == InterlockedExchange(&dwRunning, dwRunning) )
	{
		uiCurrTime = GetTimeMilliSec();
		
		//
		{
			SafeLock lock(UserSessionMgr._Lock);

			for( CUserSession *pSession : UserSessionMgr._UsedUserSessionList )
			{
				pSession->DoUpdate(uiCurrTime);
			}
		}

		//
		Sleep(1);
	}

	//
	return 0;
}

unsigned int WINAPI CommandThread(void *p)
{
	Network &Net = Network::GetInstance();

	//
	DWORD &dwRunning = Net._dwRunning;
	INT64 biCurrTime = 0;

	//
	char cmd[512+1] = {};
	CConnector *pConnector = nullptr;
	CUserSession *pUserSession = nullptr;

	WCHAR wszHostIP[] = L"127.0.0.1";
	WORD wHostPort = 20010;

	g_Log.Write(L"log: CommandThread() start.");

	while( 1 == InterlockedExchange(&dwRunning, dwRunning) )
	{
		if( 1 )
		{
			gets_s(cmd, 512);
			string strCommand = cmd;
			if( 0 >= strCommand.length() )
				continue;

			vector<string> tokens;
			TokenizeA(strCommand, tokens, " ");

			if( 0 == strncmp(tokens[0].c_str(), "/connect", tokens[0].length()) )
			{
				pConnector = Net.Connect(HOST_ADDRESS, HOST_PORT);
				if( !pConnector )
				{
					g_Log.Write(L"info: connect fail.");
				}
				else
				{
					g_Log.Write(L"info: connected.");

					pUserSession = CUserSessionMgr::GetInstance().GetFreeUserSession();
					if( !pUserSession )
					{
						g_Log.Write(L"error: CUserSessionMgr::GetFreeUserSession() fail");
						Net.Disconnect(pConnector);
					}
					else
					{
						pConnector->SetParam(pUserSession);
						pUserSession->SetConnector(pConnector);
					}
				}
			}
			else if( 0 == strncmp(tokens[0].c_str(), "/disconnect", tokens[0].length()) )
			{
				if( pConnector )
					Net.Disconnect(pConnector);
			}
			else if( 0 == strncmp(tokens[0].c_str(), "/auth", tokens[0].length()) )
			{
				if( pUserSession ) pUserSession->ReqAuth();
			}
			else if( 0 == strncmp(tokens[0].c_str(), "/send", tokens[0].length()) )
			{
				if( pUserSession ) pUserSession->ReqEcho();
			}
			else if( 0 == strncmp(tokens[0].c_str(), "/sendloop", tokens[0].length()) )
			{
				int loopcount = 5;
				if( 2 <= tokens.size() )
					loopcount = atoi(tokens[1].c_str());
				if( 1 > loopcount )
					loopcount = 1;

				for( int cnt = 0; cnt < loopcount; ++cnt )
				{
					if( pUserSession ) 
						pUserSession->ReqEcho();
					Sleep(500);
				}
			}
			else if( 0 == strncmp(tokens[0].c_str(), "/exit", tokens[0].length()) )
			{
				Net.Stop();
				break;
			}
		}

		//
		if( 0 )
		{
			// connect
			pConnector = Net.Connect(HOST_ADDRESS, HOST_PORT);
			if( !pConnector )
			{
				g_Log.Write(L"info: connect fail.");
			}
			else
			{
				g_Log.Write(L"info: connected.");

				CUserSession *pUserSession = CUserSessionMgr::GetInstance().GetFreeUserSession();
				if( !pUserSession )
				{
					g_Log.Write(L"error: CUserSessionMgr::GetFreeUserSession() fail");
					Net.Disconnect(pConnector);
				}
				else
				{
					pConnector->SetParam(pUserSession);
					pUserSession->SetConnector(pConnector);
				}
			}

			Sleep(10);

			// auth
			{
				struct
				{
					DWORD dwLength;
					DWORD dwProtocol;
					sP_AUTH Data;
				} SendData;
				memset((void*)&SendData, 0, sizeof(SendData));

				SendData.dwLength = sizeof(SendData);
				SendData.dwProtocol = P_AUTH;

				if( pConnector )
					pConnector->AddSendData((char*)&SendData, sizeof(SendData));
			}

			Sleep(10);

			// send loop 3
			{
				struct
				{
					DWORD dwLength;
					DWORD dwProtocol;
					sP_ECHO Data;
				} SendData;
				memset((void*)&SendData, 0, sizeof(SendData));

				SendData.dwLength = sizeof(SendData);
				SendData.dwProtocol = P_ECHO;
				string echomsg = "abcdefg12345";
				memcpy(SendData.Data.echoData, echomsg.c_str(), echomsg.length());

				for( int cnt = 0; cnt < 3; ++cnt )
				{
					if( pConnector ) pConnector->AddSendData((char*)&SendData, sizeof(SendData));
					Sleep(1);
				}
			}

			// disconnect
			if( pConnector )
				Net.Disconnect(pConnector);

			Sleep(10);
		}
	}

	g_Log.Write(L"log: msgprocThread() end.");

	//
	return 0;
}