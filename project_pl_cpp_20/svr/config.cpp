#include "stdafx.h"

#include "./config.h"

//
ServerConfig serverConfig;

bool ServerConfig::SetConfig(ConfigLoader& config)
{
    id = config.GetInt("id");
    name = config.GetString("name");

    return true;
}
