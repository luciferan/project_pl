#pragma once
#ifndef __CERTIFYDATA_H__
#define __CERTIFYDATA_H__

#include "./_common.h"
//#include <Windows.h>
#include <time.h>
#include <vector>

#include "../_lib/util_Time.h"

//
namespace Certify
{
const INT32 MAX_STRING_LEN{60};
const INT32 MAX_CONVERT_BUFFER_LEN{80}; //(INT32)(MAX_STRING_LEN * 1.4);

const INT32 MAX_ENC_STRING_LEN{10};
const INT32 MAX_CHECK_COUNT{3};

typedef std::vector<char> VecConvertTable;

//
bool CertifyConvertTableInit();
int CertifyDataRecover(int nConvertTable, char* pszData);
};

//
class CCertifyData
{
public:
    int _nStringIndex{0};
    int _nConvertTableID{0};

    int _nLastSendTime{0};
    int _nCheckCount{0};

    INT32 _nEncKey{0};
    char _szString[Certify::MAX_CONVERT_BUFFER_LEN + 1]{0};

    //
public:
    CCertifyData() {}
    ~CCertifyData() {}

    size_t SetString(char* pszString);
    int SetString();
    int GetString(char szBuffer[]) { memcpy(szBuffer, _szString, Certify::MAX_CONVERT_BUFFER_LEN); return Certify::MAX_CONVERT_BUFFER_LEN; }

    int Convert(int nConvertTable);
    int Recover(int nConvertTable);

    void Encrypt(INT32 nEncKey);
    void Encrypt(INT64 ulEncKey);
    void Encrypt(char* pszEncString);

    void Decrypt(INT32 nEncKey) { Encrypt(nEncKey); }
    void Decrypt(INT64 ulEncKey) { Encrypt(ulEncKey); }
    void Decrypt(char* pszEncString) { Encrypt(pszEncString); }

    //
    void InitCertify() { _nLastSendTime = _time32(NULL) + (SEC_A_SEC * 90); }
    bool CheckSendTime(int nCheckTime);
    int GetCheckCount() { return _nCheckCount; }

    int MakeCertifyData(INT32 nEncKey = 0);
    int GetCertifyData(char szCertifyData[]);

    bool CheckCertifyData(char szData[]);
};

//
#endif //__CERTIFYDATA_H__