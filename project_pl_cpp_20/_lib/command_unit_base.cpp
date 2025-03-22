#include "./command_unit_base.h"

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
    SafeLock lock(_lock);
    _cmds.push_back(op);
}

void CommandUnitQueue::Tick()
{
    FireOps();
}

void CommandUnitQueue::SwapOpList(CommandList &firingCmds)
{
    SafeLock lock(_lock);
    firingCmds.swap(_cmds);
}

void CommandUnitQueue::FireOps()
{
    CommandList firingCmds{};
    SwapOpList(firingCmds);

    for (CommandUnitBase*it : firingCmds) {
        CommandUnitBase* cmd = it;
        ProcessAndDelete(cmd);
    }

    firingCmds.clear();
}
