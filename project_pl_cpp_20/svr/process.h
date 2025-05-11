#pragma once
#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "../_framework/network.h"
#include "../_framework/packet_data_queue.h"
#include "../_lib/exception_report.h"
#include "../_lib/util.h"

#include "./command_unit_process.h"
#include "./config.h"

#include <thread>
#include <atomic>
#include <list>
#include <set>

//
class App
{
private:
    list<thread> _threads{};

    stop_source _threadStop;
    atomic<int> _threadSuspended{1};
    atomic<int> _threadWait{0};
    INT64 _biUpdateTime{0};

    CommandUnitQueue _commandQueue;

    //
private:
    App() {}

public:
    virtual ~App() {}
    static App& GetInstance()
    {
        static App* pInstance = new App;
        return *pInstance;
    }

    bool Init(ConfigLoader& configLoader);

    bool Start();
    bool Stop();

    unsigned int ProcessThread(stop_token token);
    unsigned int UpdateThread(stop_token token);

    CommandUnitQueue& GetCmdQueue() { return _commandQueue; }
};

extern CommandUnitQueue& GetCmdQueue();

//
#endif //__PROCESS_H__
