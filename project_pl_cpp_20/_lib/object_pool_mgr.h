#pragma once
#ifndef __OBJECT_POOL_MGR_H__
#define __OBJECT_POOL_MGR_H__

#include "./safe_lock.h"

#include <string>
#include <format>
#include <numeric>
#include <vector>
#include <list>

using namespace std;

//
template <typename T, typename Allocator = allocator<T>>
class ObjectPool
{
private:
    vector<T*> _poolList;
    vector<T*> _freeList;

    Allocator _allocator;
    static const size_t _initSize{100};
    size_t _addBlockSize{_initSize};

    //
private:
    void AddAlloc()
    {
        auto* obj{_allocator.allocate(_addBlockSize)};
        _poolList.push_back(obj);

        auto oldSize{_freeList.size()};
        _freeList.resize(oldSize + _addBlockSize);
        iota(begin(_freeList) + oldSize, end(_freeList), obj);
        _addBlockSize *= 2;
    }

public:
    ObjectPool() = default;
    explicit ObjectPool(const Allocator& allocator)
        : _allocator{allocator}
    {
    }

    virtual ~ObjectPool()
    {
        size_t chunkSize{_initSize};
        for (auto* chunk : _poolList) {
            _allocator.deallocate(chunk, chunkSize);
            chunkSize *= 2;
        }
        _poolList.clear();
    }

    ObjectPool(ObjectPool& src) noexcept = default;
    ObjectPool& operator=(ObjectPool&& rhs) noexcept = default;

    ObjectPool(const ObjectPool& src) = delete;
    ObjectPool& operator=(ObjectPool& rhs) = delete;

    template<typename... Args>
    shared_ptr<T> GetFreeObject(Args... args)
    {
        if (_freeList.empty()) {
            AddAlloc();
        }

        T* object{_freeList.back()};
        new(object) T{forward<Args>(args)...};
        _freeList.pop_back();

        return shared_ptr<T> { object, [this](T* object) {
            destroy_at(object);
            _freeList.push_back(object);
        }};
    }

    template<typename... Args>
    T* GetFreeObjectPtr(Args... args)
    {
        if (_freeList.empty()) {
            AddAlloc();
        }

        T* object{_freeList.back()};
        new(object) T{forward<Args>(args)...};
        _freeList.pop_back();

        return object;
    }

    void SetFreeObjectPtr(T* object)
    {
        destroy_at(object);
        _freeList.push_back(object);
    }

    string GetReportA()
    {
        return format("pool:{}, free:{}", _poolList.size(), _freeList.size());
    }

    wstring GetReport()
    {
        return format(L"pool:{}, free:{}", _poolList.size(), _freeList.size());
    }
};

//
template <typename T, typename Allocator = allocator<T>>
class ObjectMgrBase
{
protected:
    Lock _lock;

    ObjectPool<T> _objectPool;
    list<T*> _usedList{};

public:
    ObjectMgrBase()
    {
    }

    virtual ~ObjectMgrBase()
    {
    }

    T* GetFreeObject()
    {
        SafeLock lock(_lock);
        auto obj(_objectPool.GetFreeObjectPtr());
        _usedList.emplace_back(obj);

        return obj;
    }

    T* GetFreeObject(SafeLock&)
    {
        auto obj(_objectPool.GetFreeObjectPtr());
        _usedList.emplace_back(obj);

        return obj;
    }

    void SetFreeObject(T* obj)
    {
        SafeLock lock(_lock);
        if (find(_usedList.begin(), _usedList.end(), obj) != _usedList.end()) {
            _usedList.remove(obj);
        }
        _objectPool.SetFreeObjectPtr(obj);
    }

    void SetFreeObject(SafeLock&, T* obj)
    {
        if (find(_usedList.begin(), _usedList.end(), obj) != _usedList.end()) {
            _usedList.remove(obj);
        }
        _objectPool.SetFreeObjectPtr(obj);
    }

    string GetReportA()
    {
        SafeLock lock(_lock);
        return format("{}, used:{}", _objectPool.GetReportA(), _usedList.size());
    }

    wstring GetReport()
    {
        SafeLock lock(_lock);
        return format(L"{}, used:{}", _objectPool.GetReport(), _usedList.size());
    }
};

//
template <class T>
class ObjectPoolMgrBase
{
protected:
    Lock _lock;

    list<T*> _poolList{};
    list<T*> _freeList{};
    list<T*> _usedList{};
    int _poolCount{0};

private:
    void AddAlloc()
    {
        int addPool = _poolCount * 2;

        for (int idx = 0; idx < addPool; ++idx) {
            T* obj = new T;
            _poolList.emplace_back(obj);
            _freeList.emplace_back(obj);
        }
        _poolCount += addPool;
    }

public:
    ObjectPoolMgrBase(int initCount)
        : _poolCount(initCount)
    {
        _poolCount = max(_poolCount, 1);

        for (int idx = 0; idx < _poolCount; ++idx) {
            T* obj = new T;
            _poolList.emplace_back(obj);
            _freeList.emplace_back(obj);
        }
    }

    virtual ~ObjectPoolMgrBase()
    {
        for (auto* it : _poolList) {
            delete it;
        }
        _poolList.clear();
        _freeList.clear();
        _usedList.clear();
    }

    T* GetFreeObject()
    {
        SafeLock lock(_lock);

        if (_freeList.empty()) {
            AddAlloc();
        }
        if (_freeList.empty()) {
            return nullptr;
        }

        T* obj = _freeList.front();
        _freeList.pop_front();
        _usedList.emplace_back(obj);

        return obj;
    }

    void SetFreeObject(T* obj)
    {
        SafeLock lock(_lock);

        if (find(_usedList.begin(), _usedList.end(), obj) != _usedList.end()) {
            _usedList.remove(obj);
            _freeList.emplace_front(obj);
        }
    }

    string GetReportA()
    {
        SafeLock lock(_lock);
        return format("pool:{}, used:{}, free:{}", _poolList.size(), _usedList.size(), _freeList.size());

    }

    wstring GetReport()
    {
        SafeLock lock(_lock);
        return format(L"pool:{}, used:{}, free:{}", _poolList.size(), _usedList.size(), _freeList.size());
    }
};

#endif //__OBJECT_POOL_MGR_H__