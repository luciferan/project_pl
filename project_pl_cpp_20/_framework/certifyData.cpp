#include "CertifyData.h"
#include "CertifyConvertData.h"

using namespace Certify;

//
Certify::VecConvertTable g_VecBaseTable[10];

//
bool Certify::CertifyConvertTableInit()
{
	for( int nCount = 0; nCount < 10; ++nCount )
	{
		g_VecBaseTable[nCount].clear();
		g_VecBaseTable[nCount].reserve(64);
	}

	for( int nCount = 0; nCount < 10; ++nCount )
	{
		for( int nOffset = 0; nOffset < 64; ++nOffset )
		{
			g_VecBaseTable[nCount].push_back(*(szConvertTable[nCount]+nOffset));
		}
	}

	return true;
}

int Certify::CertifyDataRecover(int nConvertTable, char *szData)
{
	int nDecBufferOffset = 0;

	char szEncBuffer[MAX_CONVERT_BUFFER_LEN + 1] = {0,}; 
	char szDecBuffer[MAX_CONVERT_BUFFER_LEN + 1] = {0,}; 
	memcpy(szEncBuffer, szData, sizeof(char) * MAX_CONVERT_BUFFER_LEN);

	for( int nOffset = 0; nOffset < MAX_CONVERT_BUFFER_LEN; nOffset += 4 )
	{
		char enc[4] = {0,};
		enc[0] = szEncBuffer[nOffset+0];
		enc[1] = szEncBuffer[nOffset+1];
		enc[2] = szEncBuffer[nOffset+2];
		enc[3] = szEncBuffer[nOffset+3];

		for( int nArr = 0; nArr < 4; ++nArr )
		{
			for( int nEncKey = 0; nEncKey < 64; ++nEncKey )
			{
				if( enc[nArr] != g_VecBaseTable[nConvertTable][nEncKey] )
					continue;

				enc[nArr] = nEncKey;
				break;
			}
		}

		//
		char dec[3] = {0,};
		dec[0] = (enc[0] << 2) | (enc[1] >> 4);
		dec[1] = (enc[1] << 4) | (enc[2] >> 2);
		dec[2] = (enc[2] << 6) | (enc[3]);

		szDecBuffer[nDecBufferOffset++] = dec[0];
		szDecBuffer[nDecBufferOffset++] = dec[1];
		szDecBuffer[nDecBufferOffset++] = dec[2];
	}

	memcpy(szData, szDecBuffer, sizeof(char) * MAX_CONVERT_BUFFER_LEN);
	return nDecBufferOffset;
}

char g_szCertifyString[10][MAX_STRING_LEN + 1] = 
{
	"123456789012345678901234567890123456789012345678901234567890",
	"234567890123456789012345678901234567890123456789012345678901",
	"345678901234567890123456789012345678901234567890123456789012",
	"456789012345678901234567890123456789012345678901234567890123",
	"567890123456789012345678901234567890123456789012345678901234",
	"678901234567890123456789012345678901234567890123456789012345",
	"789012345678901234567890123456789012345678901234567890123456",
	"890123456789012345678901234567890123456789012345678901234567",
	"901234567890123456789012345678901234567890123456789012345678",
	"012345678901234567890123456789012345678901234567890123456789",
};

//
CCertifyData::CCertifyData()
{
	_nStringIndex = 0;
	_nConvertTableID = 0;

	_nLastSendTime = 0;
	_nCheckCount = 0;

	_nEncKey = 0;
	memset(_szString, 0, sizeof(_szString));
}

CCertifyData::~CCertifyData()
{
}

size_t CCertifyData::SetString(char *pszString)
{
	if( !pszString )
		return 0;
	
	size_t nStrLen = strlen(pszString);
	if( 0 == nStrLen )
		return 0;
	if( MAX_STRING_LEN < nStrLen )
		nStrLen = MAX_STRING_LEN;

	memcpy(_szString, pszString, sizeof(char) * nStrLen);
	return nStrLen;
}

int CCertifyData::SetString()
{
	int nRand = (int)(rand() % 10);
	if( 0 > nRand )
		nRand = 0;
	if( 10 <= nRand )
		nRand = 9;

	strncpy_s(_szString, _countof(_szString), g_szCertifyString[nRand], MAX_STRING_LEN);
	return _nStringIndex = nRand;
}

int CCertifyData::Convert(int nConvertTableID)
{
	int nEncBufferOffset = 0;
	char szEncBuffer[MAX_CONVERT_BUFFER_LEN + 1] = {0,}; 

	for( int nOffset = 0; nOffset < MAX_STRING_LEN; nOffset += 3 )
	{
		char szTemp[3] = {0,};
		memcpy(szTemp, _szString+nOffset, 3);

		char enc[4] = {0,};
		enc[0] = ((szTemp[0] & 0xFC) >> 2);
		enc[1] = ((szTemp[0] & 0x03) << 4) | ((szTemp[1] & 0xF0) >> 4);
		enc[2] = ((szTemp[1] & 0x0F) << 2) | ((szTemp[2] & 0xC0) >> 6);
		enc[3] = ((szTemp[2] & 0x3F));

		//
		szEncBuffer[nEncBufferOffset++] = (g_VecBaseTable[nConvertTableID][enc[0]]);
		szEncBuffer[nEncBufferOffset++] = (g_VecBaseTable[nConvertTableID][enc[1]]);
		szEncBuffer[nEncBufferOffset++] = (g_VecBaseTable[nConvertTableID][enc[2]]);
		szEncBuffer[nEncBufferOffset++] = (g_VecBaseTable[nConvertTableID][enc[3]]);
	}

	memcpy(_szString, szEncBuffer, sizeof(char) * MAX_CONVERT_BUFFER_LEN);
	_nConvertTableID = nConvertTableID;
	return nEncBufferOffset;
}

int CCertifyData::Recover(int nConvertTableID)
{
	int nDecBufferOffset = 0;

	char szEncBuffer[MAX_CONVERT_BUFFER_LEN + 1] = {0,}; 
	char szDecBuffer[MAX_CONVERT_BUFFER_LEN + 1] = {0,}; 
	memcpy(szEncBuffer, _szString, sizeof(char) * MAX_CONVERT_BUFFER_LEN);

	for( int nOffset = 0; nOffset < MAX_CONVERT_BUFFER_LEN; nOffset += 4 )
	{
		char enc[4] = {0,};
		enc[0] = szEncBuffer[nOffset+0];
		enc[1] = szEncBuffer[nOffset+1];
		enc[2] = szEncBuffer[nOffset+2];
		enc[3] = szEncBuffer[nOffset+3];

		for( int nArr = 0; nArr < 4; ++nArr )
		{
			for( int nEncKey = 0; nEncKey < 64; ++nEncKey )
			{
				if( enc[nArr] != g_VecBaseTable[nConvertTableID][nEncKey] )
					continue;

				enc[nArr] = nEncKey;
				break;
			}
		}

		//
		char dec[3] = {0,};
		dec[0] = (enc[0] << 2) | (enc[1] >> 4);
		dec[1] = (enc[1] << 4) | (enc[2] >> 2);
		dec[2] = (enc[2] << 6) | (enc[3]);

		szDecBuffer[nDecBufferOffset++] = dec[0];
		szDecBuffer[nDecBufferOffset++] = dec[1];
		szDecBuffer[nDecBufferOffset++] = dec[2];
	}

	memcpy(_szString, szDecBuffer, sizeof(char) * MAX_CONVERT_BUFFER_LEN);
	return nDecBufferOffset;
}

void CCertifyData::Encrypt(INT32 nEncKey)
{
	for( int nOffset = 0; nOffset < MAX_CONVERT_BUFFER_LEN; nOffset += sizeof(INT32) )
	{
		INT32 nBuffer = 0;

		memcpy((void*)&nBuffer, _szString+nOffset, sizeof(INT32));
		nBuffer ^= nEncKey;
		memcpy(_szString+nOffset, (void*)&nBuffer, sizeof(INT32));
	}
}

void CCertifyData::Encrypt(INT64 ulEncKey)
{
	for( int nOffset = 0; nOffset < MAX_CONVERT_BUFFER_LEN; nOffset += sizeof(INT64) )
	{
		INT64 ulBuffer = 0;

		memcpy((void*)&ulBuffer, _szString+nOffset, sizeof(INT64));
		ulBuffer ^= ulEncKey;
		memcpy(_szString+nOffset, (void*)&ulBuffer, sizeof(INT64));
	}
}

void CCertifyData::Encrypt(char *pszEncString)
{
	if( !pszEncString )
		return;
	int nStrLen = (int)strlen(pszEncString);
	if( 0 >= nStrLen )
		return;
	if( MAX_ENC_STRING_LEN < nStrLen )
		nStrLen = MAX_ENC_STRING_LEN;
	
	for( int nOffset = 0; nOffset < MAX_CONVERT_BUFFER_LEN; nOffset += (sizeof(char) * nStrLen) )
	{
		char szBuffer[MAX_ENC_STRING_LEN + 1] = {0,};

		memcpy((void*)&szBuffer, _szString+nOffset, (sizeof(char) * nStrLen));
		for( int nPos = 0; nPos < nStrLen; ++nPos )
			szBuffer[nPos] ^= pszEncString[nPos];
		memcpy(_szString+nOffset, (void*)&szBuffer, (sizeof(char) * nStrLen));
	}	
}

//
bool CCertifyData::CheckSendTime(int nCheckTime)
{
	if( 0 >= _nLastSendTime )
		return false;

	__time32_t t32CurrTime = _time32(NULL);
	if( _nLastSendTime + nCheckTime <= t32CurrTime )
	{
		_nLastSendTime = t32CurrTime;
		return true;
	}

	return false;
}

int CCertifyData::MakeCertifyData(INT32 nEncKey)
{
	if( 0 == nEncKey )
		nEncKey = ((rand() % 10000) << 16) | (rand() % 10000);

	SetString();
	_nEncKey = nEncKey;
	Encrypt(nEncKey);

	int nRand = rand() % 10;
	Convert(nRand);

	_nCheckCount += 1;
	return _nConvertTableID;
}

int CCertifyData::GetCertifyData(char szCertifyData[])
{
	memcpy(szCertifyData, _szString, sizeof(char) * MAX_CONVERT_BUFFER_LEN);

	return _nConvertTableID;
}

bool CCertifyData::CheckCertifyData(char szData[])
{
	memcpy(_szString, szData, sizeof(char) * MAX_CONVERT_BUFFER_LEN);
	Decrypt(_nEncKey);

	if( 0 != strncmp(_szString, g_szCertifyString[_nStringIndex], strlen(g_szCertifyString[_nStringIndex])) )
		return false;

	_nCheckCount -= 1;
	return true;
}
