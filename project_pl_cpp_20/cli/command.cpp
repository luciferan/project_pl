#include "../_lib/operator.h"

#include <iostream>

using namespace std;

class TestCommandUnit : public CommandUnitBase
{
public:
    TestCommandUnit() : CommandUnitBase(ECommentUnitPriority::Normal) {}
    ~TestCommandUnit() {}

    void Operator()
    {
        cout << "call test command" << endl;
    }
};