#include "./operator.h"

static void ProcessAndDelete(CommandUnitBase* cmd)
{
    cmd->Operator();
    delete cmd;
}

CommandUnitQueue::~CommandUnitQueue()
{
    for (CommandUnitBase* it : _cmds) {
        CommandUnitBase* cmd = it;
        delete cmd;
    }
}

void CommandUnitQueue::Add(CommandUnitBase* op)
{
    SafeLock lock(_cs);
    _cmds.push_back(op);
}

void CommandUnitQueue::Tick()
{
    SwapOpList();
    FireOps();
}

void CommandUnitQueue::SwapOpList()
{
    SafeLock lock(_cs);
    _firingCmds.swap(_cmds);
}

void CommandUnitQueue::FireOps()
{
    for (CommandUnitBase*it : _firingCmds) {
        CommandUnitBase* cmd = it;
        ProcessAndDelete(cmd);
    }
    _firingCmds.clear();
}
