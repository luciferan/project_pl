#pragma once
#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "../_framework/network.h"
#include "../_lib/exception_report.h"
#include "../_lib/util.h"

#include "./command_unit_process.h"

#include <thread>
#include <atomic>
#include <list>

//
//bool LoadConfig();

//
class App
{
private:
    list<thread> _threads{};

    stop_source _threadStop;
    atomic<int> _threadSuspended{1};
    atomic<int> _threadWait{0};

    CommandUnitQueue _commandQueue;

    //
private:
    App() {}

public:
    ~App() {}
    static App& GetInstance()
    {
        static App* pInstance = new App;
        return *pInstance;
    }

    bool Start();
    bool Stop();

    unsigned int ProcessThread(stop_token token);
    unsigned int UpdateThread(stop_token token);
    unsigned int CommandThread(stop_token token);

    CommandUnitQueue& GetCmdQueue() { return _commandQueue; }
};

extern CommandUnitQueue& GetCmdQueue();

//
#endif //__PROCESS_H__
