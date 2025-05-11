#pragma once
#ifndef __CONFIG_LOADER_H__
#define __CONFIG_LOADER_H__

#include "../_external/rapidjson/document.h"

#include <string>

using namespace std;
using namespace rapidjson;

//
struct ServerInfo
{
    int id{0};
    string name{""};
    string ip{""};
    unsigned short port{0};
};

//
class ConfigLoader
{
private:
    Document _doc;

public:
    ConfigLoader();
    ~ConfigLoader();

    bool LoadConfig(string path);

    int GetInt(string key, int defaultValue = 0);
    bool GetBool(string key, bool defaultValue = false);
    string GetString(string key, string defaultValue = "");

    bool GetServerInfo(string key, ServerInfo& serverInfo);
};

//
#endif // __CONFIG_LOADER_H__
