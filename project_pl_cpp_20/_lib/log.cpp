#include "Log.h"
#include "util_common.h"
#include "util_String.h"
#include "util_Time.h"

#include <io.h>
#include <iostream>

//
void Log(const std::string& str)
{
    g_Log.Write(eLogLevel::Normal, FormatA("[%-8s] %s", "normal", str.c_str()));
}

void Log(const std::wstring& wstr)
{
    g_Log.Write(eLogLevel::Normal, FormatW(L"[%-8s] %s", L"normal", wstr.c_str()));
}

void LogDebug(const std::string& str)
{
    g_Log.Write(eLogLevel::Debug, FormatA("[%-8s] %s", "debug", str.c_str()));
}

void LogDebug(const std::wstring& wstr)
{
    g_Log.Write(eLogLevel::Debug, FormatW(L"[%-8s] %s", L"debug", wstr.c_str()));
}

void LogInfo(const std::string& str)
{
    g_Log.Write(eLogLevel::Info, FormatA("[%-8s] %s", "info", str.c_str()));
}

void LogError(const std::string& str, std::source_location loc /*= std::source_location::current()*/)
{
    g_Log.Write(eLogLevel::Error, FormatA("[%-8s] %s %s", "error", str.c_str(), format("Loc:{} {} {}", loc.file_name(), loc.function_name(), loc.line()).c_str()));
}

void PacketLog(const std::string& str, const char* pPacketData, int nPacketDataSize)
{
    std::string strBuffer{};
    strBuffer.append(str);

    if (1024 < nPacketDataSize) {
        for (int idx = 0; idx < 6; ++idx) {
            strBuffer.append(format("{0:02X}", pPacketData[idx]));
        }
        strBuffer.append(". connot write packetlog. too long.");

    } else {
        for (int idx = 0; idx < nPacketDataSize; ++idx) {
            strBuffer.append(format("{0:02X}", pPacketData[idx]));
        }
        strBuffer.append("\0");
    }

    LogDebug(strBuffer);
}

void PerformanceLog(const string& str)
{
    g_PerformanceLog.Write(eLogLevel::Info, FormatA("[%8s] %s", "performance", str.c_str()));
}

void PerformanceLog(const wstring& wstr)
{
    g_PerformanceLog.Write(eLogLevel::Info, FormatW(L"[%8s] %s", L"performance", wstr.c_str()));
}

//
FileLogger g_Log;
FileLogger g_PerformanceLog;

const WCHAR* pwcsLogFilePath{L"log"};

//
FileLogger::FileLogger()
{
    CreateDirectoryW(pwcsLogFilePath, NULL);
    _wstrFileName = GetFileName();
}

FileLogger::~FileLogger()
{
    Flush();
}

void FileLogger::SetLogFile(wstring wstrFileName)
{
    _wstrFileName = wstrFileName;
}

void FileLogger::SetLogLevel(eLogLevel logLevel)
{
    _logLevel = logLevel;

}

void FileLogger::Write(eLogLevel logLevel, const string& str)
{
    CTimeSet CurrTime;
    wstring wstrLogText = FormatW(L"%02d:%02d:%02d %hs", CurrTime.GetHour(), CurrTime.GetMin(), CurrTime.GetSec(), str.c_str());

    AddMessage(wstrLogText);
    ConsoleWrite(logLevel, wstrLogText);
}

void FileLogger::Write(eLogLevel logLevel, const std::wstring& wstr)
{
    CTimeSet CurrTime;
    wstring wstrLogText = FormatW(L"%02d:%02d:%02d %s", CurrTime.GetHour(), CurrTime.GetMin(), CurrTime.GetSec(), wstr.c_str());

    AddMessage(wstrLogText);
    ConsoleWrite(logLevel, wstrLogText);
}

//void FileLogger::Write(eLogLevel logLevel, const WCHAR* pFormat, ...)
//{
//    WCHAR* pwcsBuffer = new WCHAR[MAX_LEN_LOG_STRING + 1];
//    memset(pwcsBuffer, 0, sizeof(pwcsBuffer));
//
//    va_list args;
//    va_start(args, pFormat);
//    _vsnwprintf_s(pwcsBuffer, MAX_LEN_LOG_STRING, MAX_LEN_LOG_STRING, pFormat, args);
//    va_end(args);
//
//    //
//    CTimeSet CurrTime;
//    wstring wstrLogText = FormatW(L"%02d:%02d:%02d %s", CurrTime.GetHour(), CurrTime.GetMin(), CurrTime.GetSec(), pwcsBuffer);
//
//    AddMessage(wstrLogText);
//    ConsoleWrite(logLevel, wstrLogText);
//}

void FileLogger::AddMessage(const std::wstring& wstr)
{
    if (_maxQueueSize <= 1) {
        return FileWrite(wstr.c_str());
    }

    {
        SafeLock msgLock(_msgLock);
        _msgQueue.push_back(wstr);
    }

    Flush();
}

void FileLogger::Flush(bool bForce /*= false*/)
{
    if (bForce == false && _msgQueue.size() < _maxQueueSize) {
        return;
    }

    list<wstring> list{};
    {
        SafeLock msgLock(_msgLock);
        list.swap(_msgQueue);
    }

    SafeLock fileLock(_fileLock);
    FileWrite(fileLock, list);
}

void FileLogger::ConsoleWrite(eLogLevel logLevel, const wstring& wstr)
{
    if (logLevel < _logLevel) {
        return;
    }

    wprintf(L"%s\n", wstr.c_str());
}

void FileLogger::FileWrite(const wstring& wstr)
{
    SafeLock lock(_fileLock);
    FileWrite(lock, list<wstring>{wstr});
}

void FileLogger::FileWrite(SafeLock& lock, const list<wstring> &list)
{
    CTimeSet CurrTime;
    wstring wstrFileName{};

    switch (_eDivideType) {
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
    FILE* pFile{nullptr};

    auto error{_waccess_s(wstrFileName.c_str(), 0x02)};
    if (0 != error) {
        error = _wfopen_s(&pFile, wstrFileName.c_str(), L"w");
        if (0 != error) {
            ConsoleWrite(eLogLevel::Error, FormatW(L"error: FileLogger::FileWrite(): log file open fail %d", error).c_str());
            return;
        }
    } else {
        error = _wfopen_s(&pFile, wstrFileName.c_str(), L"a");
        if (0 != error) {
            ConsoleWrite(eLogLevel::Error, FormatW(L"error: FileLogger::FileWrite(): log file open fail %d", error).c_str());
            return;
        }
    }

    if (pFile) {
        for (wstring wstr : list) {
            fwprintf_s(pFile, L"%s\n", wstr.c_str());
        }
        fclose(pFile);
    }
}