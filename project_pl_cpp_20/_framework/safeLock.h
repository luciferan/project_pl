#pragma once
#ifndef __SAFELOCK_H__
#define __SAFELOCK_H__

//
#include <windows.h>

//
class Lock
{
private:
	CRITICAL_SECTION _cs;
public:
	Lock() { InitializeCriticalSectionAndSpinCount(&_cs, 2000); }
	~Lock() { DeleteCriticalSection(&_cs); }

	void Locked() { EnterCriticalSection(&_cs); }
	void Unlock() { LeaveCriticalSection(&_cs); }
};

class ScopeLock
{
private:
	Lock *_Lock = nullptr;
public:
	ScopeLock(Lock &lock) { _Lock = &lock; _Lock->Locked(); }
	~ScopeLock() { _Lock->Unlock(); _Lock = nullptr; }
};

class SafeLock
{
private:
	LPCRITICAL_SECTION _cs = nullptr;
public:
	SafeLock(CRITICAL_SECTION& cs) { _cs = &cs; EnterCriticalSection(_cs); }
	~SafeLock() { LeaveCriticalSection(_cs); _cs = nullptr; }
};


//
#endif //__SAFELOCK_H__