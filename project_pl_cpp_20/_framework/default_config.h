#pragma once
#ifndef __DEFAULT_CONFIG_H__
#define __DEFAULT_CONFIG_H__

#include "./_common.h"
#include "../_lib/util_common.h"
#include <string>

using namespace std;

//
class IniFile
{
public:
    wstring _wstrConfigFileName{};
    WCHAR _wcsFileName[1024 + 1]{0,};

public:
    IniFile();
    virtual ~IniFile();
    void SetConfigFile(wstring wstrFileName);

    void GetValue(LPCWSTR appName, LPCWSTR keyName, WORD& wValue, WORD wDefaultValue = -1);
    void GetValue(LPCWSTR appName, LPCWSTR keyName, INT& nValue, INT nDefaultValue = -1);
    void GetValue(LPCWSTR appName, LPCWSTR keyName, INT64& biValue, INT64 biDefaultValue = -1);
    void GetValue(LPCWSTR appName, LPCWSTR keyName, LPWSTR pwcsResult, DWORD dwResultLen);
};

//
class DefaultConfig
    : public IniFile
{
public:
    DefaultConfig() {};
    virtual ~DefaultConfig() {};
};

//
#endif //__DEFAULT_CONFIG_H__