#include "../_external/rapidjson/document.h"
#include "../_external/rapidjson/stringbuffer.h"
#include "../_external/rapidjson/filereadstream.h"

#include "./config_loader.h"

#include <fstream>
#include <string>

//
ConfigLoader::ConfigLoader()
{
}

ConfigLoader::~ConfigLoader()
{
}

bool ConfigLoader::LoadConfig(string path)
{
    bool ret = true;

    ifstream ifs(path);
    try {
        if (!ifs.is_open()) {
            ret = false;
            throw runtime_error("failed to open file");
        }

        string json((istreambuf_iterator<char>(ifs)), (istreambuf_iterator<char>()));
        //Document doc;
        _doc.Parse(json.c_str());

        if (!_doc.IsObject()) {
            ret = false;
            throw runtime_error("json is not an object");
        }
    }
    catch (exception e) {

    }

    return ret;
}
int ConfigLoader::GetInt(string key, int defaultValue /*= 0*/)
{
    if (_doc.HasMember(key.c_str()) && _doc[key.c_str()].IsInt()) {
        return _doc[key.c_str()].GetInt();
    }

    return defaultValue;
}

bool ConfigLoader::GetBool(string key, bool defaultValue /*= false*/)
{
    if (_doc.HasMember(key.c_str()) && _doc[key.c_str()].IsBool()) {
        return _doc[key.c_str()].GetBool();
    }
    return defaultValue;
}

string ConfigLoader::GetString(string key, string defaultValue /*= ""*/)
{
    if (_doc.HasMember(key.c_str()) && _doc[key.c_str()].IsString()) {
        return _doc[key.c_str()].GetString();
    }
    return defaultValue;
}

bool ConfigLoader::GetServerInfo(string key, ServerInfo& serverInfo)
{
    if (_doc.HasMember(key.c_str())) {
        if (_doc[key.c_str()].HasMember("server_info")) {
            if (_doc[key.c_str()]["server_info"].HasMember("id") && _doc[key.c_str()]["server_info"]["id"].IsInt()) {
                serverInfo.id = _doc[key.c_str()]["server_info"]["id"].GetInt();
            }
            if (_doc[key.c_str()]["server_info"].HasMember("name") && _doc[key.c_str()]["server_info"]["name"].IsString()) {
                serverInfo.name = _doc[key.c_str()]["server_info"]["name"].GetString();
            }
            if (_doc[key.c_str()]["server_info"].HasMember("ip") && _doc[key.c_str()]["server_info"]["ip"].IsString()) {
                serverInfo.ip = _doc[key.c_str()]["server_info"]["ip"].GetString();
            }
            if (_doc[key.c_str()]["server_info"].HasMember("port") && _doc[key.c_str()]["server_info"]["port"].IsInt()) {
                serverInfo.port = _doc[key.c_str()]["server_info"]["port"].GetInt();
            }
        }
        
        return true;
    }

    return false;
}

//bool LoadConfig(string path)
//{
//    bool ret = true;
//
//    ifstream ifs(path);
//
//    try {
//        if (!ifs.is_open()) {
//            ret = false;
//            throw runtime_error("failed to open file");
//        }
//
//        string json((istreambuf_iterator<char>(ifs)), (istreambuf_iterator<char>()));
//        Document doc;
//        doc.Parse(json.c_str());
//
//        if (!doc.IsObject()) {
//            ret = false;
//            throw runtime_error("json is not an object");
//        }
//
//        if (doc.HasMember("id") && doc["id"].IsInt() ) {
//            serverConfig.id = doc["id"].GetInt();
//        } else {
//            serverConfig.id = 0;
//        }
//
//        if (doc.HasMember("name") && doc["name"].IsString()) {
//            serverConfig.name = doc["name"].GetString();
//        } else {
//            serverConfig.name = "";
//        }
//
//        if (doc.HasMember("client_session")) {
//            if (doc["client_session"].HasMember("listen_info")) {
//                if (doc["client_session"]["listen_info"].HasMember("ip") && doc["client_session"]["listen_info"]["ip"].IsString()) {
//                    serverConfig.listenClient.strIP = doc["client_session"]["listen_info"]["ip"].GetString();
//                }
//                if (doc["client_session"]["listen_info"].HasMember("port") && doc["client_session"]["listen_info"]["port"].IsInt()) {
//                    serverConfig.listenClient.wPort = doc["client_session"]["listen_info"]["port"].GetInt();
//                }
//            }
//        }
//
//        if (doc.HasMember("stream_session")) {
//            if (doc["stream_session"].HasMember("server_info")) {
//                if (doc["stream_session"]["listen_info"].HasMember("ip") && doc["stream_session"]["listen_info"]["ip"].IsString()) {
//                    serverConfig.listenStream.strIP = doc["stream_session"]["listen_info"]["ip"].GetString();
//                }
//                if (doc["stream_session"]["listen_info"].HasMember("port") && doc["stream_session"]["listen_info"]["port"].IsInt()) {
//                    serverConfig.listenStream.wPort = doc["stream_session"]["listen_info"]["port"].GetInt();
//                }
//            }
//        }
//
//        if (doc.HasMember("web_session")) {
//            if (doc["web_session"].HasMember("server_info")) {
//                if (doc["web_session"]["listen_info"].HasMember("ip") && doc["web_session"]["listen_info"]["ip"].IsString()) {
//                    serverConfig.listenWeb.strIP = doc["web_session"]["listen_info"]["ip"].GetString();
//                }
//                if (doc["web_session"]["listen_info"].HasMember("port") && doc["web_session"]["listen_info"]["port"].IsInt()) {
//                    serverConfig.listenWeb.wPort = doc["web_session"]["listen_info"]["port"].GetInt();
//                }
//            }
//        }
//
//        if (doc.HasMember("db_server")) {
//            if (doc["db_server"].HasMember("connect_info")) {
//                if (doc["db_server"]["connect_info"].HasMember("ip") && doc["db_server"]["connect_info"]["ip"].IsString()) {
//                    serverConfig.DBServer.strIP = doc["db_server"]["connect_info"]["ip"].GetString();
//                }
//                if (doc["db_server"]["connect_info"].HasMember("port") && doc["db_server"]["connect_info"]["port"].IsInt()) {
//                    serverConfig.DBServer.wPort = doc["db_server"]["connect_info"]["port"].GetInt();
//                }
//            }
//        }
//
//    }
//    catch (exception e) {
//        
//    }
//
//    return ret;
//}