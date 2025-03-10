#include "../_lib/command_unit_base.h"

#include <iostream>

using namespace std;

class TestCommandUnit : public CommandUnitBase
{
public:
    TestCommandUnit() : CommandUnitBase(ECommandUnitPriority::Normal) {}
    ~TestCommandUnit() {}

    void Operator()
    {
        cout << "call test command" << endl;
    }
};