#pragma once
#ifndef __UTIL_TIME_H__
#define __UTIL_TIME_H__

//
#include <windows.h>
#include <time.h>
#include <sys/timeb.h>
#include <sqltypes.h>

#include <iostream>
#include <string>
#include <format>
#include <source_location>
#include <chrono>

using namespace std;

//
const INT32 SEC_A_SEC = 1;
const INT32 SEC_A_MIN = SEC_A_SEC * 60;
const INT32 SEC_A_HOUR = SEC_A_MIN * 60;
const INT32 SEC_A_DAY = SEC_A_HOUR * 24;
const INT32 SEC_A_WEEK = SEC_A_DAY * 7;
const INT32 SEC_A_YEAR = SEC_A_DAY * 365;

const INT64 MILLISEC_A_SEC = 1000;
const INT64 MILLISEC_A_MIN = MILLISEC_A_SEC * 60;
const INT64 MILLISEC_A_HOUR = MILLISEC_A_MIN * 60;
const INT64 MILLISEC_A_DAY = MILLISEC_A_HOUR * 24;
const INT64 MILLISEC_A_WEEK = MILLISEC_A_DAY * 7;
const INT64 MILLISEC_A_YEAR = MILLISEC_A_DAY * 365;

INT32 GetTimeSec();
INT64 GetTimeMilliSec();

//
class CTimeSet
{
private:
	__time64_t _tGMT;
	__time64_t _tLocalT;

	struct tm _tmGMT;
	struct tm _tmLocalT;

	//
public:
	CTimeSet();
	CTimeSet(__time64_t tTime, bool bGMT);
	CTimeSet(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec, bool bGMT);
	CTimeSet(TIMESTAMP_STRUCT dbTime);

	void SetTime();
	void SetTime(__time64_t tTime, bool bGMT);
	void SetTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec, bool bGMT);
	void SetTime(TIMESTAMP_STRUCT dbTime);

	__time64_t ConvertLC2GM(__time64_t tLocal);
	__time64_t ConvertLC2GM(struct tm tmLocal);

	__time64_t GetTime_GM() { return _tGMT; }
	__time64_t GetTime_LC() { return _tLocalT; }
	struct tm GetTimeTM_GM() { return _tmGMT; }
	struct tm GetTimeTM_LC() { return _tmLocalT; }
	__time64_t GetTime(bool local = true) { if (local) return _tLocalT; else return _tGMT; }
	struct tm GetTimeTM(bool local = true) { if (local) return _tmLocalT; else return _tmGMT; }

	//
	int GetYear(bool bLocal = true) { return (bLocal ? _tmLocalT.tm_year : _tmGMT.tm_year) + 1900; }
	int GetMonth(bool bLocal = true) { return (bLocal ? _tmLocalT.tm_mon + 1 : _tmGMT.tm_mon + 1); }
	int GetDay(bool bLocal = true) { return (bLocal ? _tmLocalT.tm_mday : _tmGMT.tm_mday); }
	int GetHour(bool bLocal = true) { return (bLocal ? _tmLocalT.tm_hour : _tmGMT.tm_hour); }
	int GetMin(bool bLocal = true) { return (bLocal ? _tmLocalT.tm_min : _tmGMT.tm_min); }
	int GetSec(bool bLocal = true) { return (bLocal ? _tmLocalT.tm_sec : _tmGMT.tm_sec); }

	//
	void _test();
};

//
class PerformanceChecker {
private:
	chrono::high_resolution_clock::time_point _start;
	chrono::high_resolution_clock::time_point _end;
	source_location loc;

public:
	PerformanceChecker(source_location loc = source_location::current()) : _start(chrono::high_resolution_clock::now()) 
	{
	}
	
	~PerformanceChecker() 
	{
		_end = chrono::high_resolution_clock::now();

		chrono::milliseconds span = chrono::duration_cast<chrono::milliseconds>(_end - _start);
		cout << format("Percormance {} {}: {}", loc.function_name(), loc.line(), span) << endl;
	}
};

//
#endif //__UTIL_TIME_H__