#include "ExceptionReport.h"
#include "util.h"

#include <string>
#include <source_location>

using namespace std;

CExceptionReport::CExceptionReport()
{
    _hDebugHelp = LoadLibraryW(L"dbghelp.dll");
}

CExceptionReport::~CExceptionReport()
{
}

DWORD CExceptionReport::ExceptionHandlerBegin()
{
    SetUnhandledExceptionFilter(makeFullDump);
    return 0;
}

LONG WINAPI CExceptionReport::makeFullDump(LPEXCEPTION_POINTERS lpExceptionPointer)
{
    //return makeDump(lpExceptionPointer, MiniDumpWithFullMemory);
    return makeDump(lpExceptionPointer, MiniDumpNormal);
}

LONG WINAPI CExceptionReport::makeDump(LPEXCEPTION_POINTERS lpExceptionPointer, MINIDUMP_TYPE dumpType)
{
    CTimeSet currTime;

    wstring fileName = GetFileName();
    wstring dumpFileName{format(L"{}_{:04d}-{:02d}-{:02d}_{:02d}-{:02d}-{:02d}.dmp",  fileName.c_str(), currTime.GetYear(), currTime.GetMonth(), currTime.GetDay(), currTime.GetHour(), currTime.GetMin(), currTime.GetSec())};

    //
    HANDLE hFile = CreateFile(dumpFileName.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE != hFile) {
        MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
        exceptionInfo.ThreadId = GetCurrentThreadId();
        exceptionInfo.ExceptionPointers = lpExceptionPointer;
        exceptionInfo.ClientPointers = NULL;

        auto bRet = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, dumpType, &exceptionInfo, NULL, NULL);
        if (!bRet) {
            auto err = GetLastError();
        }

        CloseHandle(hFile);
    } else {
        auto err = GetLastError();
    }

    //
    return EXCEPTION_EXECUTE_HANDLER;
}
