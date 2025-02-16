#include "./util_time.h"
#include <iostream>
#include <format>

INT32 GetTimeSec()
{
	struct __timeb32 tTime;
	_ftime32_s(&tTime);
	return (INT32)(tTime.time);
}

INT64 GetTimeMilliSec()
{
	struct __timeb64 tTime;
	_ftime64_s(&tTime);
	return (INT64)(tTime.time * 1000 + tTime.millitm);
}

//
CTimeSet::CTimeSet()
{
	SetTimeZone();
	SetTime();
}

CTimeSet::CTimeSet(__time64_t tTime, bool bLocal /*= default_local_time*/)
{
	SetTimeZone();
	SetTime(tTime, bLocal);
}

CTimeSet::CTimeSet(initializer_list<int> yyMMddhhmmss, bool bLocal /*= default_local_time*/)
{
	SetTimeZone();
	SetTime(yyMMddhhmmss, bLocal);
}

CTimeSet::CTimeSet(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec, bool bLocal /*= default_local_time*/)
{
	SetTimeZone();
	SetTime(nYear, nMonth, nDay, nHour, nMin, nSec, bLocal);
}

CTimeSet::CTimeSet(TIMESTAMP_STRUCT dbTimeStamp, bool bLocal /*= default_local_time*/)
{
	SetTimeZone();
	SetTime(dbTimeStamp.year, dbTimeStamp.month, dbTimeStamp.day, dbTimeStamp.hour, dbTimeStamp.minute, dbTimeStamp.second, bLocal);
}

void CTimeSet::SetTimeZone()
{
	_tzset();
	_get_timezone(&_tzSec); // KST -32400
}

void CTimeSet::SetTime()
{
	_time64(&_tGMT);
	_gmtime64_s(&_tmGMT, &_tGMT);
	_localtime64_s(&_tmLocalT, &_tGMT);
	_tLocalT = _mkgmtime64(&_tmLocalT);
}

void CTimeSet::SetTime(__time64_t tTime, bool bLocal /*= default_local_time*/)
{
	struct tm* ptmTime = nullptr;

	if (bLocal) {
		_tGMT = tTime + _tzSec;
		_tLocalT = tTime;
		_gmtime64_s(&_tmGMT, &_tGMT);
		_gmtime64_s(&_tmLocalT, &_tLocalT);
	} else {
		_tGMT = tTime;
		_tLocalT = tTime - _tzSec;
		_gmtime64_s(&_tmGMT, &_tGMT);
		_gmtime64_s(&_tmLocalT, &_tLocalT);
	}
}

void CTimeSet::SetTime(initializer_list<int> yyMMddhhmmss, bool bLocal /*= default_local*/)
{
	vector<int> t = {1900, 1, 0, 0, 0, 0};

	struct tm tmTime = {};
	auto it = yyMMddhhmmss.begin();
	for (size_t idx = 0; idx < yyMMddhhmmss.size() || idx < 6; ++idx) {
		t[idx] = *it++;
	}

	tmTime.tm_year = t[0] - 1900;
	tmTime.tm_mon = t[1] - 1;
	tmTime.tm_mday = t[2];
	tmTime.tm_hour = t[3];
	tmTime.tm_min = t[4];
	tmTime.tm_sec = t[5];
	tmTime.tm_isdst = -1;

	struct tm* ptmTime = nullptr;

	if (bLocal) {
		__time64_t tTime = _mkgmtime64(&tmTime);
		_tGMT = tTime + _tzSec;
	} else {
		_tGMT = _mktime64(&tmTime);
	}

	_gmtime64_s(&_tmGMT, &_tGMT);
	localtime_s(&_tmLocalT, &_tGMT);
	_tLocalT = _mkgmtime64(&_tmLocalT);
}

void CTimeSet::SetTime(int nYear, int nMon, int nDay, int nHour, int nMin, int nSec, bool bLocal /*= default_local*/)
{
	struct tm tmTime = {};

	tmTime.tm_year = nYear - 1900;
	tmTime.tm_mon = nMon - 1;
	tmTime.tm_mday = nDay;
	tmTime.tm_hour = nHour;
	tmTime.tm_min = nMin;
	tmTime.tm_sec = nSec;
	tmTime.tm_isdst = -1;

	struct tm* ptmTime = nullptr;

	if (bLocal) {
		__time64_t tTime = _mkgmtime64(&tmTime);
		_tGMT = tTime + _tzSec;
	} else {
		_tGMT = _mktime64(&tmTime);
	}

	_gmtime64_s(&_tmGMT, &_tGMT);
	localtime_s(&_tmLocalT, &_tGMT);
	_tLocalT = _mkgmtime64(&_tmLocalT);
}

void CTimeSet::SetTime(TIMESTAMP_STRUCT dbTimeStamp, bool bLocal /*= default_local_time*/)
{
	SetTime({dbTimeStamp.year, dbTimeStamp.month, dbTimeStamp.day, dbTimeStamp.hour, dbTimeStamp.minute, dbTimeStamp.second}, bLocal);
}

__time64_t CTimeSet::ConvertLC2GM(__time64_t tTime)
{
	return tTime + _tzSec;
}

__time64_t CTimeSet::ConvertLC2GM(struct tm tmLocal)
{
	__time64_t tTime = _mkgmtime64(&tmLocal);
	return tTime + _tzSec;
}

void CTimeSet::print()
{
	cout << format("gmt: {}", _tGMT) << endl;
	cout << format("gmt: {}-{}-{} {:02d}:{:02d}:{:02d}", _tmGMT.tm_year + 1900, _tmGMT.tm_mon + 1, _tmGMT.tm_mday, _tmGMT.tm_hour, _tmGMT.tm_min, _tmGMT.tm_sec) << endl;
	cout << format("local: {}", _tLocalT) << endl;
	cout << format("local: {}-{}-{} {:02d}:{:02d}:{:02d}", _tmLocalT.tm_year + 1900, _tmLocalT.tm_mon + 1, _tmLocalT.tm_mday, _tmLocalT.tm_hour, _tmLocalT.tm_min, _tmLocalT.tm_sec) << endl;
}

void CTimeSet::_test()
{
	// time 함수 테스트
	{
		struct tm tm_gmt, tm_local;
		__time64_t time_gmt = 0, time_local = 0, time_gap = 0;

		time_gmt = _time64(NULL);
		_gmtime64_s(&tm_gmt, &time_gmt);
		_localtime64_s(&tm_local, &time_gmt);
		time_local = _mkgmtime64(&tm_local);
		time_gap = time_local - time_gmt;

		wprintf(L"%d-%d-%d %d:%d:%d\n", tm_gmt.tm_year + 1900, tm_gmt.tm_mon + 1, tm_gmt.tm_mday, tm_gmt.tm_hour, tm_gmt.tm_min, tm_gmt.tm_sec);
		wprintf(L"%d-%d-%d %d:%d:%d\n", tm_local.tm_year + 1900, tm_local.tm_mon + 1, tm_local.tm_mday, tm_local.tm_hour, tm_local.tm_min, tm_local.tm_sec);
		wprintf(L"gmt:%I64d, local:%I64d, gap:%I64d(%I64dh)\n", time_gmt, time_local, time_gap, time_gap / 60 / 60);
	}

	// ctimeset 테스트
	{
		CTimeSet currTime;

		__time64_t time_gmt = currTime.GetTime_GM();
		__time64_t time_local = currTime.GetTime_LC();
		__time64_t time_gap = time_local - time_gmt;
		struct tm tm_gmt = currTime.GetTimeTM_GM();
		struct tm tm_local = currTime.GetTimeTM_LC();

		wprintf(L"%d-%d-%d %d:%d:%d\n", tm_gmt.tm_year + 1900, tm_gmt.tm_mon + 1, tm_gmt.tm_mday, tm_gmt.tm_hour, tm_gmt.tm_min, tm_gmt.tm_sec);
		wprintf(L"%d-%d-%d %d:%d:%d\n", tm_local.tm_year + 1900, tm_local.tm_mon + 1, tm_local.tm_mday, tm_local.tm_hour, tm_local.tm_min, tm_local.tm_sec);
		wprintf(L"gmt:%I64d, local:%I64d, gap:%I64d(%I64dh)\n", time_gmt, time_local, time_gap, time_gap / 60 / 60);
	}

	{
		CPerformanceChecker p_check;

		CTimeSet time1;
		time1.print();

		time1.SetTime({2025, 2, 14, 9, 0, 0}, true);
		time1.print();

		auto check_gmt = time1.GetTime(false);
		auto check_local = time1.GetTime();

		CTimeSet time2(check_gmt, false);
		time2.print();

		CTimeSet time3(check_local);
		time3.print();
	}
}

//
CPerformanceChecker::CPerformanceChecker(source_location loc /*= source_location::current()*/)
	: _loc(loc)
{
}

CPerformanceChecker::CPerformanceChecker(string* pstrBuffer, source_location loc /*= source_location::current()*/)
	: _pstrBuffer(pstrBuffer), _loc(loc)
{
}

CPerformanceChecker::~CPerformanceChecker()
{
	chrono::steady_clock::time_point end = chrono::steady_clock::now();
	chrono::nanoseconds processTime = end - _start;

	if (_pstrBuffer) {
		*_pstrBuffer = format("func:{} [line:{}]: process time: {}", _loc.function_name(), _loc.line(), processTime.count());
	} else {
		cout << format("func:{} [line:{}]: process time: {}", _loc.function_name(), _loc.line(), processTime.count()) << endl;
	}
}

void unit_test_time()
{
	CTimeSet t1;
	t1._test();
}
