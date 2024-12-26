#pragma once
#ifndef __LOG_H__
#define __LOG_H__

//
#include <windows.h>
#include <wchar.h>
#include <string>

#include "safeLock.h"

//
using namespace std;

const int MAX_LEN_LOG_STRING = 8192;

enum class eLog
{
	
};

enum class eLogDivideType
{
	Day,
	Hour,
	Min,
};

//
class CLog
{
private:
	wstring _wstrFileName = {};
	eLogDivideType _eDivideType = eLogDivideType::Day;

	Lock _FileLock;
	FILE *_pFile = nullptr;


	//
private:
	void ConsoleWrite(const WCHAR *pwcsLogText);
	void FileWrite(const WCHAR *pwcsLogText);

public:
	CLog();
	virtual ~CLog();

	bool Set(wstring wstrFileName);

	void Write(std::wstring wstr);
	void Write(const WCHAR *pwcsFormat, ...);

};

//
extern CLog g_Log;
extern CLog g_PerformanceLog;

//
#endif //__LOG_H__