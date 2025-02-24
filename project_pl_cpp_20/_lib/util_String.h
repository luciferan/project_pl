#pragma once
#ifndef __UTIL_STRING_H__
#define __UTIL_STRING_H__

//
#include <windows.h>

#include <algorithm>
#include <string>
#include <vector>

//
using namespace std;

enum eString
{
    BUFFER_MAX = 1024 * 10,
};

//
string FormatA(const char* pFormat, ...);
wstring FormatW(const WCHAR* pFormat, ...);

void TokenizeA(string& str, vector<string>& tokens, string delimiter);
void TokenizeW(wstring& str, vector<wstring>& tokens, wstring delimiter);

//
#endif //__UTIL_STRING_H__