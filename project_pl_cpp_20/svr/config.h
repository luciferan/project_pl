#pragma once
#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "../_lib/config_loader.h"

//
struct ServerConfig
{
    int id{0};
    string name{""};

    bool SetConfig(ConfigLoader& config);
};

extern ServerConfig serverConfig;

//
#endif //__CONFIG_H__