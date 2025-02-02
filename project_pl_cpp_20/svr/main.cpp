#include "process.h"

#include <iostream>

extern void Log(const std::wstring& wstr);
extern void Log(const std::string& str);

//
int main(int argc, char* argv[])
{
	CExceptionReport::GetInstance().ExceptionHandlerBegin();

	//
	//LoadConfig();

	//
	Log("system: svr start.");

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

	Log("system: svr end.");

	//
	return 0;
}