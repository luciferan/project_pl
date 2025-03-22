#include "process.h"

//
int main(int argc, char* argv[])
{
    CExceptionReport::GetInstance().ExceptionHandlerBegin();

    srand((unsigned int)time(NULL));

    //
    //LoadConfig();

    //
    g_Log.Write(L"system: start.");
    App::GetInstance().Start();
    g_Log.Write(L"system: end.");
    return 0;
}
