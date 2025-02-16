#pragma once
#ifndef __UTIL_TIME_H__
#define __UTIL_TIME_H__

//
#include <windows.h>
#include <time.h>
#include <sys/timeb.h>
#include <sqltypes.h>

#include <string>
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

const bool default_local_time = true;
//
class CTimeSet
{
private:
	long _tzSec = 0;
	__time64_t _tGMT = 0;
	__time64_t _tLocalT = 0;

	struct tm _tmGMT = {};
	struct tm _tmLocalT = {};

	//
public:
	CTimeSet();
	CTimeSet(__time64_t tTime, bool bLocal = default_local_time);
	CTimeSet(initializer_list<int> yyMMddhhmmss, bool bLocal = default_local_time);
	CTimeSet(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec, bool bLocal = default_local_time);
	CTimeSet(TIMESTAMP_STRUCT dbTime, bool bLocal = default_local_time);

	void SetTimeZone();

	void SetTime();
	void SetTime(__time64_t tTime, bool bLocal = default_local_time);
	void SetTime(initializer_list<int> yyMMddhhmmss, bool bLocal = default_local_time);
	void SetTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec, bool bLocal = default_local_time);
	void SetTime(TIMESTAMP_STRUCT dbTime, bool bLocal = default_local_time);

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
	void print();
	void _test();
};
void unit_test_time();

//
class CPerformanceChecker
{
private:
	chrono::steady_clock::time_point _start = chrono::steady_clock::now();
	source_location _loc;

	string* _pstrBuffer = nullptr;

public:
	CPerformanceChecker(source_location loc = source_location::current());
	CPerformanceChecker(string* pstrBuffer, source_location loc = source_location::current());
	~CPerformanceChecker();
};

//
#endif //__UTIL_TIME_H__