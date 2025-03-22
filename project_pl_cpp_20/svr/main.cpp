#include "stdafx.h"
#include "./process.h"

#include <iostream>

extern void Log(const std::wstring& wstr);
extern void Log(const std::string& str);

//
int main(int argc, char* argv[])
{
    CExceptionReport::GetInstance().ExceptionHandlerBegin();
    srand((unsigned int)time(NULL));

    //
    //LoadConfig();

    //
    Log("system: svr start.");

    App& app = App::GetInstance();
    if (app.Init()) {
        app.Start();
    } else {
        return 1;
    }

    //
    char cmd[1028 + 1] {0,};
    while (1) {
        std::cin >> cmd;
        if (0 == strcmp(cmd, "/exit")) {
            break;
        }
    }

    app.Stop();

    //
    Log("system: svr end.");
    return 0;
}