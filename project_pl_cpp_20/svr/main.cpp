#include "process.h"

#include <iostream>

//
int main(int argc, char* argv[])
{
	CExceptionReport::GetInstance().ExceptionHandlerBegin();

	//
	//LoadConfig();

	//
	Network &net = Network::GetInstance();

	//
	DWORD &dwRunning = net._dwRunning;
	//unsigned int uiThreadID = 0;
	//HANDLE hUpdateThread = (HANDLE)_beginthreadex(NULL, 0, UpdateThread, NULL, CREATE_SUSPENDED, &uiThreadID);
	//HANDLE hProcessThread = (HANDLE)_beginthreadex(NULL, 0, ProcessThread, NULL, CREATE_SUSPENDED, &uiThreadID);
	//HANDLE hMonitorThread = (HANDLE)_beginthreadex(NULL, 0, MonitorThread, NULL, CREATE_SUSPENDED, &uiThreadID);

	//
	g_Log.Write(L"system: start.");

	//WCHAR wcsHostIP[] = L"127.0.0.1";
	//WORD wHostPort = 20010;

	//net._config.doListen = true;
	//wcsncpy_s(net._config.listenInfo.wcsIP, eNetwork::MAX_LEN_IP4_STRING, wcsHostIP, lstrlenW(wcsHostIP));
	//net._config.listenInfo.wPort = 20010;

	//if (false == net.Initialize()) {
	//	return 1;
	//}

	//if (false == net.Start()) {
	//	return 1;
	//}

	//ResumeThread(hUpdateThread);
	//ResumeThread(hProcessThread);
	//ResumeThread(hMonitorThread);
	App app;

	if (app.Init()) {
		app.Run();
	} else {
		return 1;
	}

	//
	char cmd[1028 + 1] = { 0, };
	//while (1 == InterlockedExchange((LONG*)&dwRunning, dwRunning)) {
	while (1) {
		//gets_s(cmd);
		std::cin >> cmd;
		if (0 == strcmp(cmd, "/exit")) {
			break;
		}

		//Sleep(500);
	}

	app.Stop();

	//
	//net.Stop();
	//WaitForSingleObject(hUpdateThread, INFINITE);
	CRecvPacketQueue::GetInstance().ForceActivateQueueEvent();
	//WaitForSingleObject(hProcessThread, INFINITE);
	//WaitForSingleObject(hMonitorThread, INFINITE);

	g_Log.Write(L"system: end.");

	//
	return 0;
}