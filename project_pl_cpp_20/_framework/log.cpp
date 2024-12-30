#include "Log.h"
#include "util_common.h"
#include "util_String.h"
#include "util_Time.h"

#include <iostream>

//
CLog g_Log;
CLog g_PerformanceLog;

const WCHAR *pwcsLogFilePath = L"log";



//
CLog::CLog()
{
	CreateDirectoryW(pwcsLogFilePath, NULL);

	_wstrFileName = GetFileName();
}

CLog::~CLog()
{
}

bool CLog::Set(wstring wstrFileName)
{
	_wstrFileName = wstrFileName;
	return true;
}

void CLog::Write(std::wstring wstr)
{
	CTimeSet CurrTime;
	//wprintf(L"%d:%d:%d %s\n", CurrTime.GetHour(), CurrTime.GetMin(), CurrTime.GetSec(), wstr.c_str());
	wstring wstrLogText = FormatW(L"%02d:%02d:%02d %s", CurrTime.GetHour(), CurrTime.GetMin(), CurrTime.GetSec(), wstr.c_str());
	FileWrite(wstrLogText.c_str());
	wprintf(L"%s\n", wstrLogText.c_str());

	//
	return;
}

void CLog::Write(const WCHAR *pFormat, ...)
{
	WCHAR buffer[MAX_LEN_LOG_STRING + 1] = { 0, };

	va_list args;
	va_start(args, pFormat);
	_vsnwprintf_s(buffer, _countof(buffer), _countof(buffer), pFormat, args);
	va_end(args);

	//
	CTimeSet CurrTime;
	//wprintf(L"%d:%d:%d %s\n", CurrTime.GetHour(), CurrTime.GetMin(), CurrTime.GetSec(), buffer);
	wstring wstrLogText = FormatW(L"%02d:%02d:%02d %s", CurrTime.GetHour(), CurrTime.GetMin(), CurrTime.GetSec(), buffer);
	FileWrite(wstrLogText.c_str());
	wprintf(L"%s\n", wstrLogText.c_str());

	//
	return;
}

void CLog::ConsoleWrite(const WCHAR *pwcsLogText)
{
	wprintf(L"%s\n", pwcsLogText);
}

void CLog::FileWrite(const WCHAR *pwcsLogText)
{
	CTimeSet CurrTime;
	wstring wstrFileName = {};

	SafeLock lock(_FileLock);
	
	switch( _eDivideType )
	{
	case eLogDivideType::Day:
		wstrFileName = FormatW(L"%s\\%s_%04d-%02d-%02d.log", pwcsLogFilePath, _wstrFileName.c_str(), CurrTime.GetYear(), CurrTime.GetMonth(), CurrTime.GetDay());
		break;
	case eLogDivideType::Hour:
		wstrFileName = FormatW(L"%s\\%s_%04d-%02d-%02d_%02d.log", pwcsLogFilePath, _wstrFileName.c_str(), CurrTime.GetYear(), CurrTime.GetMonth(), CurrTime.GetDay(), CurrTime.GetHour());
		break;
	case eLogDivideType::Min:
		wstrFileName = FormatW(L"%s\\%s_%04d-%02d-%02d_%02d-%02d.log", pwcsLogFilePath, _wstrFileName.c_str(), CurrTime.GetYear(), CurrTime.GetMonth(), CurrTime.GetDay(), CurrTime.GetHour(), CurrTime.GetMin());
		break;
	}

	//
	FILE *pFile = nullptr;

	auto error = _waccess_s(wstrFileName.c_str(), 0x02);
	if( 0 != error )
	{
		error = _wfopen_s(&pFile, wstrFileName.c_str(), L"w");
		if( 0 != error )
		{
			ConsoleWrite(FormatW(L"error: CLog::FileWrite(): log file open fail %d", error).c_str());
			return;
		}
	}
	else
	{
		error = _wfopen_s(&pFile, wstrFileName.c_str(), L"a");
		if( 0 != error )
		{
			ConsoleWrite(FormatW(L"error: CLog::FileWrite(): log file open fail %d", error).c_str());
			return;
		}
	}

	if( pFile )
	{
		fwprintf_s(pFile, L"%s\n", pwcsLogText);

		//
		fclose(pFile);
	}
}