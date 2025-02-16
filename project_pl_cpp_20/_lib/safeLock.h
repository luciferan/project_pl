#pragma once
#ifndef __SAFELOCK_H__
#define __SAFELOCK_H__

//#define __CRITICAL_SECTION_LOCK__
#define __MUTEX_LOCK__

//
#ifdef __CRITICAL_SECTION_LOCK__
#include <windows.h>

//
class Lock
{
private:
	CRITICAL_SECTION _cs;

public:
	Lock() { InitializeCriticalSectionAndSpinCount(&_cs, 2000); }
	~Lock() { DeleteCriticalSection(&_cs); }

	CRITICAL_SECTION& Get() { return _cs; }

	void Locked() { EnterCriticalSection(&_cs); }
	void Unlock() { LeaveCriticalSection(&_cs); }
};

class SafeLock
{
private:
	Lock& _lock;

public:
	SafeLock(Lock& lock) : _lock(lock)  { EnterCriticalSection(&_lock.Get()); }
	~SafeLock() { LeaveCriticalSection(&_lock.Get()); }
};
#endif //__CRITICAL_SECTION_LOCK__

//
#ifdef __MUTEX_LOCK__
#include <mutex>
#include <shared_mutex>

using namespace std;

class Lock
{
private:
	shared_mutex _lock;

public:
	Lock() {}
	Lock(const Lock&) = delete;
	virtual ~Lock() {}

	shared_mutex& Get() { return _lock; }
};

//
class SafeLock
{
private:
	Lock& _lock;

	bool _readOnly{ false };
	shared_lock<shared_mutex> _readLock;
	unique_lock<shared_mutex> _writeLock;

public:
	SafeLock(Lock& lock, bool readOnly = false) : _lock(lock), _readOnly(readOnly), _readLock(_lock.Get(), defer_lock_t{}), _writeLock(_lock.Get(), defer_lock_t{})
	{
		if (_readOnly) {
			_readLock.lock();
		} else {
			_writeLock.lock();
		}
	}
	~SafeLock() {
		if (_readOnly) {
			_readLock.unlock();
		} else {
			_writeLock.unlock();
		}
	}
};
#endif //__MUTEX_LOCK_

//
#endif //__SAFELOCK_H__