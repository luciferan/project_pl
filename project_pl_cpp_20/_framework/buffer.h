#pragma once
#ifndef __BUFFER_H__
#define __BUFFER_H__

//
#include <winsock2.h>
#include <WS2tcpip.h>

#include "safeLock.h"

#include <string>

//
class CConnector;

//
const int MAX_PACKET_BUFFER_SIZE = 1024 * 100;

//
class CBuffer
{
public:
	char *_pBuffer = nullptr;
	ULONG _nDataSize = 0;

protected:
	ULONG _nBufferSize = 0;

	char *_pHead = nullptr;
	char *_pTail = nullptr;
	char *_pRead = nullptr;
	char *_pWrite = nullptr;

	//
public:
	CBuffer(ULONG size = MAX_PACKET_BUFFER_SIZE)
	{
		_pHead = _pBuffer = new char[size + 1];
		_pTail = &_pBuffer[size];
		_pRead = _pWrite = _pHead;

		_nBufferSize = size;
		_nDataSize = 0;
	}

	virtual ~CBuffer()
	{
		_nBufferSize = _nDataSize = 0;
		_pHead = _pTail = _pRead = _pWrite = nullptr;

		if( _pBuffer )
			delete[] _pBuffer;
		_pBuffer = nullptr;
	}

	void Reset() 
	{ 
		_nDataSize = 0;
		_pRead = _pWrite = _pHead;
		memset(_pBuffer, 0, _nBufferSize);
	}

	char* GetBuffer() { return _pBuffer; }
	ULONG GetBufferSize() { return _nBufferSize; }
	ULONG GetDataSize() { return _nDataSize; }
	ULONG GetEmptySize() { return _nBufferSize - _nDataSize; }
};

//
class CCircleBuffer
	: public CBuffer
{
private:
	CRITICAL_SECTION _cs;

public:
	CCircleBuffer() { InitializeCriticalSectionAndSpinCount(&_cs, 2000); }
	virtual ~CCircleBuffer() { DeleteCriticalSection(&_cs); }

	ULONG Write(char IN *pSrc, ULONG IN nLen)
	{
		SafeLock lock(_cs);

		ULONG empty = _nBufferSize - _nDataSize;
		if (_nDataSize > _nBufferSize || empty < nLen)
			return -1;

		INT64 gap = _pTail - _pWrite;
		if( gap < nLen )
		{
			memcpy(_pWrite, pSrc, gap);

			_pWrite = _pHead;
			memcpy(_pWrite, pSrc + gap, nLen - gap);

			_pWrite += (nLen - gap);
			_nDataSize += nLen;
		}
		else
		{
			memcpy(_pWrite, pSrc, nLen);

			_pWrite += nLen;
			_nDataSize += nLen;
		}

		return _nDataSize;
	}

	ULONG Read(char OUT *pDest, ULONG IN nReqSize)
	{
		SafeLock lock(_cs);

		if( _nDataSize < nReqSize )
			return -1;

		char *pMark = _pRead;

		INT64 gap = _pTail - pMark;
		if( gap < nReqSize )
		{
			memcpy(pDest, pMark, gap);

			pMark = _pHead;
			memcpy(pDest+gap, pMark, nReqSize - gap);
		}
		else
		{
			memcpy(pDest, _pRead, nReqSize);
		}

		return nReqSize;
	}

	ULONG Erase(ULONG IN releaseSize)
	{
		SafeLock lock(_cs);

		if( _nDataSize < releaseSize )
			return -1;
		
		INT64 gap = _pTail - _pRead;
		if( gap < releaseSize )
		{
			_pRead = _pHead + (releaseSize - gap);
			_nDataSize -= releaseSize;
		}
		else
		{
			_pRead += releaseSize;
			_nDataSize -= releaseSize;
		}

		return _nDataSize;
	}
};

//
enum eNetworkBuffer
{
	OP_SEND = 1,
	OP_RECV = 2,
	OP_INNER = 3,
};

class CNetworkBuffer
	: public CBuffer
{
public:
	WSABUF _WSABuffer = {0,};
	eNetworkBuffer _eOperator;

	CConnector *_pSession = nullptr;

	//
public:
	CNetworkBuffer() {}
	virtual ~CNetworkBuffer() {}

	WSABUF& GetWSABuffer() { return _WSABuffer; }
	eNetworkBuffer GetOperator() { return _eOperator; }

	int SetSendData(CConnector *pSession, char *pData, UINT32 nDataSize)
	{
		if( !pData || _nBufferSize < (ULONG)nDataSize )
			return -1;

		memcpy_s(_pBuffer, _nBufferSize, pData, nDataSize);
		_nDataSize = nDataSize;

		_WSABuffer.buf = _pBuffer;
		_WSABuffer.len = _nDataSize;
		_eOperator = eNetworkBuffer::OP_SEND;
		_pSession = pSession;

		return _nDataSize;
	}

	int SetRecvData(CConnector *pSession)
	{
		_nDataSize = 0;

		_WSABuffer.buf = _pBuffer;
		_WSABuffer.len = _nBufferSize;
		_eOperator = eNetworkBuffer::OP_RECV;
		_pSession = pSession;

		return _nBufferSize;
	}

	int SetInnerData(CConnector *pSession, char *pData, UINT32 nDataSize)
	{
		if( !pData || _nBufferSize < (ULONG)nDataSize )
			return -1;

		_eOperator = eNetworkBuffer::OP_INNER;

		memcpy_s(_pBuffer, _nBufferSize, pData, nDataSize);
		_nDataSize = nDataSize;

		_pSession = pSession;
		_WSABuffer.buf = _pBuffer;
		_WSABuffer.len = _nDataSize;

		return _nDataSize;
	}
};

//
#endif //__BUFFER_H__