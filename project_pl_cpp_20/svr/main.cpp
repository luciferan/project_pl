#include "stdafx.h"

#include "./process.h"
#include "./config.h"

#include <iostream>

//
int main(int argc, char* argv[])
{
    CExceptionReport::GetInstance().ExceptionHandlerBegin();
    srand((unsigned int)time(NULL));

    string configPath{"config_svr.json"};

    for (int idx = 0; idx < argc; ++idx) {
        string param{argv[idx]};

        if (0 == param.compare("-config") && idx + 1 < argc) {
            configPath = argv[++idx];
        }
    }

    //
    ConfigLoader configLoader;
    if (!configLoader.LoadConfig(configPath)) {
        LogError("config load failed");
        return 1;
    }
    if (!serverConfig.SetConfig(configLoader)) {
        LogError("config load failed");
        return 1;
    }

    //
    Log("system: svr start.");

    App& app = App::GetInstance();
    if (app.Init(configLoader)) {
        app.Start();
    } else {
        return 1;
    }

    //
    char cmd[1028 + 1]{0,};
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