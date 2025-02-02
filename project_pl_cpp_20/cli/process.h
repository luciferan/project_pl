#pragma once
#ifndef __PROCESS_H__
#define __PROCESS_H__

//
#include "../_framework/network.h"
#include "../_lib/ExceptionReport.h"
#include "../_lib/util.h"

#include <thread>
#include <atomic>
#include <list>

//
//bool LoadConfig();

class App {
private:
	atomic<int> _threadSuspended{ 1 };
	atomic<int> _threadWait{ 1 };
	stop_source _threadStop;
	
	list<jthread> _threads;


public:
	App() {}
	~App() {}

	bool Init();
	bool Start();
	void Wait();
	bool Stop();

	int ProcessThread(stop_token token);
	int UpdateThread(stop_token token);

	int CommandThread(stop_token token);
};

//
#endif //__PROCESS_H__
