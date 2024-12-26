#pragma once
#ifndef __PROCESS_H__
#define __PROCESS_H__

//
#include "../_framework/network.h"
#include "../_framework/PacketDataQueue.h"
#include "../_framework/ExceptionReport.h"
#include "../_framework/util.h"

#include "../_framework/Log.h"
//#include "./Config.h"

#include <set>

//
//bool LoadConfig();

//
class App
{
public:
	std::set<HANDLE> _threadHandleSet{};

	DWORD _dwRunning = 0;
	INT64 _biUpdateTime = 0;

	
public:
	App();
	virtual ~App();

	bool Init();

	void Run();
	void Stop();
	
	static unsigned int WINAPI UpdateThread(void* p);
	static unsigned int WINAPI ProcessThread(void* p);
	static unsigned int WINAPI MonitorThread(void* p);


	bool SessionRelease(INT64 biCurrTime);
};

//
#endif //__PROCESS_H__
