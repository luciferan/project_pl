#pragma once
#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "../_lib/config_loader.h"

//
struct ClientConfig
{
    int id{0};
    string userName{""};
    string userPass{""};

    ServerInfo serverInfo{};

    bool SetConfig(ConfigLoader& config);
};

extern ClientConfig clientConfig;

//
#endif //__CONFIG_H__