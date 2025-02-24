#include "process.h"

//
int main(int argc, char* argv[])
{
    CExceptionReport::GetInstance().ExceptionHandlerBegin();

    //
    //LoadConfig();

    //
    g_Log.Write(L"system: start.");
    App app;
    app.Run();
    g_Log.Write(L"system: end.");
    return 0;
}
