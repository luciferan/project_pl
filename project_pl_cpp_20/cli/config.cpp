#include "stdafx.h"

#include "./config.h"

//
ClientConfig clientConfig;

bool ClientConfig::SetConfig(ConfigLoader& config)
{
    id = config.GetInt("id", 1);
    userName = config.GetString("user_name");
    userPass = config.GetString("user_pass");

    if (!config.GetServerInfo("server_info", serverInfo)) {
        return false;
    }

    return true;
}