#pragma once
#ifndef __UTIL_FILEMANAGEMENT_H__
#define __UTIL_FILEMANAGEMENT_H__

//
#include <windows.h>

#include <string>
#include <vector>
#include <functional>
#include <algorithm>

using namespace std;

//
int GetFileList(wstring wstrDirectoryPath, vector<wstring> &vecFileList);

int MakeMD5(wstring wstrFilePath, wstring &wstrChecksum);
bool CheckMD5(wstring wstrFilePath, wstring wstrChecksum);

//
#endif //__UTIL_FILEMANAGEMENT_H__