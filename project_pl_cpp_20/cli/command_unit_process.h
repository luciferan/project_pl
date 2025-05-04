#pragma once
#ifndef __COMMAND_UNIT_PROCESS_H__
#define __COMMAND_UNIT_PROCESS_H__

#include "../_lib/command_unit_base.h"
#include "../_lib/util.h"

#include "./packet_cli.h"

//
class TestCommandUnit : public CommandUnitBase
{
public:
    TestCommandUnit() 
        : CommandUnitBase(eCommandUnitPriority::Normal)
    {
    }
    void Operator();
};

//
#endif //__COMMAND_UNIT_PROCESS_H__