#pragma once
#ifndef __NETWORK_H__
#define __NETWORK_H__

//
#pragma comment(lib, "ws2_32.lib")

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <process.h>
#include <wchar.h>

#include <queue>
#include <string>

#include <set>

#include "./_common_variable.h"

class CConnector;

struct HostInfo {
	WCHAR wcsIP[eNetwork::MAX_LEN_IP4_STRING + 1] = { 0, };
	WORD wPort = 0;
};

struct NetworkConfig {
	int workerThreadCount = 4;
	bool doListen = true;

	HostInfo listenInfo;
};

//
class Network
{
public:
	NetworkConfig _config;

	HANDLE _hIOCP = INVALID_HANDLE_VALUE;

	//HANDLE _hAcceptThread = INVALID_HANDLE_VALUE;
	//HANDLE _hWorkerThread[eNetwork::MAX_THREAD_COUNT] = {INVALID_HANDLE_VALUE,};
	//HANDLE _hUpdateThread = INVALID_HANDLE_VALUE;
	std::set<HANDLE> _threadHandleSet{};

	//
	//char _szListenIP[eNetwork::MAX_LEN_IP4_STRING + 1] = {0,};
	//WCHAR _wcsListenIP[eNetwork::MAX_LEN_IP4_STRING + 1] = {0,};
	//WORD _wListenPort = 0;

	SOCKET _ListenSock = INVALID_SOCKET;
	SOCKADDR_IN _ListenAddr = {0,};

	//
	DWORD _dwRunning = 0;
	INT64 _biUpdateTime = 0;

	//
private:
	Network(void);
	~Network(void);

public:
	static Network& GetInstance()
	{
		static Network*pInstance = new Network;
		return *pInstance;
	}

	bool Initialize();
	bool Finalize();

	bool Start();
	bool Stop();

	static unsigned int WINAPI AcceptThread(void *p);
	static unsigned int WINAPI WorkerThread(void *p);
	static unsigned int WINAPI UpdateThread(void *p);

	eResultCode DoUpdate(INT64 biCurrTime);

	bool lookup_host(const char *hostname, std::string &hostIP);
	CConnector* Connect(WCHAR * pwcszIP, const WORD wPort);

	bool Disconnect(CConnector *pSession);
	bool Disconnect(SOCKET socket);
	
	eResultCode Write(CConnector *pSession, char *pSendData, int iSendDataSize);
	eResultCode InnerWrite(CConnector *pSession, char *pSendData, int nSendDataSize);
};

//
extern void WriteMiniNetLog(std::wstring wstr);
extern void WritePacketLog(std::wstring str, const char *pPacketData, int nPacketDataSize);

//
#endif //__NETWORK_H__