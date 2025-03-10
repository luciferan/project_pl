#include "stdafx.h"
#include "./default_config.h"

//
CIniFile::CIniFile()
{
    //_wstrConfigFileName = GetFileName();
    _wstrConfigFileName.append(L"./config/");
    _wstrConfigFileName.append(GetFileName());
    _wstrConfigFileName.append(L".ini");
}

CIniFile::~CIniFile()
{
}

void CIniFile::SetConfigFile(wstring wstrFileName)
{
    _wstrConfigFileName = wstrFileName;
}

void CIniFile::GetValue(LPCWSTR appName, LPCWSTR keyName, INT& nValue, INT nDefaultValue /*= -1*/)
{
    nValue = GetPrivateProfileIntW(appName, keyName, nDefaultValue, _wstrConfigFileName.c_str());
    return;
}

void CIniFile::GetValue(LPCWSTR appName, LPCWSTR keyName, WORD& wValue, WORD wDefaultValue /*= -1*/)
{
    int tempValue = wDefaultValue;
    tempValue = GetPrivateProfileIntW(appName, keyName, wDefaultValue, _wstrConfigFileName.c_str());
    if (USHRT_MAX < tempValue) {
        wValue = wDefaultValue;
    } else {
        wValue = (WORD)tempValue;
    }

    return;
}

void CIniFile::GetValue(LPCWSTR appName, LPCWSTR keyName, INT64& biValue, INT64 biDefaultValue /*= -1*/)
{
    WCHAR wcsTempString[25 + 1] = {};
    DWORD dwTempStringLen = 25;

    GetPrivateProfileStringW(appName, keyName, L"", wcsTempString, dwTempStringLen, _wstrConfigFileName.c_str());
    dwTempStringLen = wcslen(wcsTempString);
    if (20 < dwTempStringLen) { //INT64_MIN "-9223372036854775807"
        biValue = biDefaultValue;
    } else {
        biValue = _wtoi64(wcsTempString);
    }

    return;
}

void CIniFile::GetValue(LPCWSTR appName, LPCWSTR keyName, LPWSTR pwcsResult, DWORD dwResultLen)
{
    GetPrivateProfileStringW(appName, keyName, L"", pwcsResult, dwResultLen, _wstrConfigFileName.c_str());
    return;
}
