#pragma once
#ifndef __LOG_H__
#define __LOG_H__

//
#include <windows.h>
#include <wchar.h>

#include <string>
#include <format>
#include <source_location>

#include "safe_lock.h"

#include <list>

//
using namespace std;

const int MAX_LEN_LOG_STRING = 8192;

enum class eLogDivideType
{
    Day,
    Hour,
    Min,
};

enum class eLogLevel
{
    Debug,
    Info,
    Normal,
    Warning,
    Error,
    Crirical,
};

//
class FileLogger
{
private:
    wstring _wstrFileName{};
    eLogDivideType _eDivideType{eLogDivideType::Day};

    Lock _fileLock;
    FILE* _pFile{nullptr};

    Lock _msgLock;
    list<wstring> _msgQueue{};
    int _maxQueueSize{20};

    eLogLevel _logLevel{eLogLevel::Debug};

    //
private:
    void ConsoleWrite(eLogLevel logLevel, const wstring& wstr);
    void FileWrite(const wstring& wstr);
    void FileWrite(SafeLock& lock, const list<wstring>& list);

public:
    FileLogger();
    virtual ~FileLogger();

    void SetLogFile(wstring wstrFileName);
    void SetLogLevel(eLogLevel logLevel);

    void Write(eLogLevel logLevel, const std::string& str);
    void Write(eLogLevel logLevel, const std::wstring& wstr);
    //void Write(eLogLevel logLevel, const WCHAR* pwcsFormat, ...);

    void AddMessage(const std::wstring& wstr);
    void Flush(bool bForce = false);
};

//
extern FileLogger g_Log;
extern FileLogger g_PerformanceLog;

extern void Log(const std::string& str);
extern void Log(const std::wstring& str);
extern void LogDebug(const std::string& str);
extern void LogDebug(const std::wstring& wstr);
extern void LogError(const std::string& str, std::source_location loc = std::source_location::current());

extern void PacketLog(const std::string& str, const char* pPacketData, int nPacketDataSize);
extern void PerformanceLog(const std::string& str);
extern void PerformanceLog(const std::wstring& wstr);

//
#endif //__LOG_H__