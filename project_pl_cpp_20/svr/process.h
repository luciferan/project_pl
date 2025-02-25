#pragma once
#ifndef __PROCESS_H__
#define __PROCESS_H__

//
#include "../_framework/network.h"
#include "../_framework/packet_data_queue.h"
#include "../_lib/exception_report.h"
#include "../_lib/util.h"

#include "../_lib/Log.h"
//#include "./Config.h"

#include "../_lib/operator.h"

#include <thread>
#include <mutex>
#include <atomic>
#include <list>
#include <set>


//
//bool LoadConfig();

//
class App
{
public:
    list<thread> _threads{};

    stop_source _threadStop;
    atomic<int> _threadSuspended{1};
    atomic<int> _threadWait{0};
    INT64 _biUpdateTime{0};

    CommandUnitQueue _commandQueue;

    //
public:
    App();
    virtual ~App();

    bool Init();

    bool Start();
    bool Stop();

    unsigned int UpdateThread(stop_token token);
    unsigned int ProcessThread(stop_token token);
    unsigned int MonitorThread(stop_token token);

    bool SessionRelease(INT64 biCurrTime);
};

//
#endif //__PROCESS_H__
