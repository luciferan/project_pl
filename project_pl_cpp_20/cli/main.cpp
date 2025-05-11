#include "stdafx.h"

#include "./process.h"
#include "./config.h"

#include <iostream>

//
int main(int argc, char* argv[])
{
    CExceptionReport::GetInstance().ExceptionHandlerBegin();
    srand((unsigned int)time(NULL));

    string configPath{"config_cli.json"};

    for (int idx = 0; idx < argc; ++idx) {
        string param{argv[idx]};

        if (0 == param.compare("-config") && idx + 1 < argc) {
            configPath = argv[++idx];
        }
    }

    //
    //LoadConfig();
    ConfigLoader configLoader;
    if (!configLoader.LoadConfig(configPath)) {
        LogError("config load failed");
        return 1;
    }

    if (!clientConfig.SetConfig(configLoader)) {
        LogError("config load failed");
        return 1;
    }

    //
    Log("system: start.");

    if( App::GetInstance().Init(configLoader)){
        App::GetInstance().Start();
    }

    App::GetInstance().Stop();

    //
    Log("system: end.");
    return 0;
}
