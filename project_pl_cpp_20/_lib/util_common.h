#pragma once
#ifndef __UTIL_COMMON_H__
#define __UTIL_COMMON_H__

//
#include <windows.h>
#include <wchar.h>

#include <string>

//
std::wstring GetFileName();
INT64 GetUniqueSerialId();

//
#endif //__UTIL_COMMON_H__