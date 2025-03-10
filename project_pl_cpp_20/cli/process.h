#pragma once
#ifndef __PROCESS_H__
#define __PROCESS_H__

//
#include "../_framework/network.h"
#include "../_lib/exception_report.h"
#include "../_lib/util.h"
#include "../_lib/command_unit_base.h"

#include <thread>
#include <atomic>
#include <list>

//
//bool LoadConfig();

class App
{
private:
    list<thread> _threads{};

    stop_source _threadStop;
    atomic<int> _threadSuspended{1};
    atomic<int> _threadWait{0};

    CommandUnitQueue _commandQueue;

    //
public:
    App() {}
    ~App() {}

    bool Run();
    bool Stop();

    int ProcessThread(stop_token token);
    int UpdateThread(stop_token token);
    int CommandThread(stop_token token);
};

//
#endif //__PROCESS_H__
