#pragma once
#ifndef __COMMAND_UNIT_BASE_H__
#define __COMMAND_UNIT_BASE_H__

#include "./safe_lock.h"

#include <functional>
#include <vector>

using namespace std;

enum class eCommandUnitPriority
{
    Low = 0,
    Normal,
    Highest,
    MAX
};

class CommandUnitBase
{
private:
    eCommandUnitPriority _priority;

public:
    CommandUnitBase(eCommandUnitPriority priority)
        : _priority(priority)
    {
    }
    virtual ~CommandUnitBase() {}
    virtual void Operator() = 0;
};

class DynamicCommandUnit : public CommandUnitBase
{
public:
    typedef function<void()> Operation;
private:
    Operation _operation;

public:
    DynamicCommandUnit(const Operation& operation)
        : CommandUnitBase(eCommandUnitPriority::Normal)
        , _operation(operation)
    {
    }

    virtual void Operator()
    {
        if (_operation) {
            _operation();
        }
    }
};

class CommandUnitQueue
{
private:
    Lock _lock;

    typedef std::vector<CommandUnitBase*> CommandList;
    CommandList _cmds{};

public:
    CommandUnitQueue() {}
    virtual ~CommandUnitQueue();

    void Add(CommandUnitBase* op);
    void Tick();

private:
    void SwapOpList(CommandList &firingCmd);
    void FireOps();
};

//
#endif //__COMMAND_UNIT_BASE_H__