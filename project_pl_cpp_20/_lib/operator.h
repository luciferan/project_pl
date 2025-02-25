#pragma once
#ifndef __OPERATOR_H__
#define __OPERATOR_H__

#include "./safeLock.h"

#include <functional>
#include <vector>

using namespace std;

enum class ECommentUnitPriority
{
    Low = 0,
    Normal,
    Highest,
    MAX
};

class CommandUnitBase
{
private:
    ECommentUnitPriority _priority;

public:
    CommandUnitBase(ECommentUnitPriority priority)
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
        : CommandUnitBase(ECommentUnitPriority::Normal)
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
    Lock _cs;

    typedef std::vector<CommandUnitBase*> CommandList;
    CommandList _cmds{};
    CommandList _firingCmds{};

public:
    CommandUnitQueue() {}
    virtual ~CommandUnitQueue();

    void Add(CommandUnitBase* op);
    void Tick();

private:
    void SwapOpList();
    void FireOps();
};
#endif //__OPERATOR_H__