#pragma once
#ifndef __CONNECTOR_H__
#define __CONNECTOR_H__

//
#include <winsock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

#include <unordered_map>
#include <queue>
#include <list>

#include "./buffer.h"

#include "./safeLock.h"
#include "./util_String.h"

#include "./_common_variable.h"

//
using namespace std;
class CConnector;

//
struct OVERLAPPED_EX
{
	OVERLAPPED overlapped = {0,};

	CConnector *pSession = nullptr;
	CNetworkBuffer *pBuffer = nullptr;

	//
	void ResetOverlapped() { memset(&overlapped, 0, sizeof(OVERLAPPED)); }
};

//
class CConnector 
{
protected:
	DWORD _dwUniqueIndex = 0;
	//DWORD _dwUsed = 0;
	//DWORD _dwActive = 0;

	void *_pParam = nullptr;

	SOCKET _Socket = INVALID_SOCKET;

public:
	SOCKADDR_IN _SockAddr = {0,};

	char _szDomain[eNetwork::MAX_LEN_DOMAIN_STRING + 1] = {0,};
	WCHAR _wcsDomain[eNetwork::MAX_LEN_DOMAIN_STRING + 1] = {0,};
	WORD _wPort = 0;

	// send
	OVERLAPPED_EX _SendRequest; 
	Lock _SendQueueLock;
	queue<CNetworkBuffer*> _SendQueue = {}; // 여기있는걸 send 합니다. 최대 갯수를 넘어가면 전송량에 비해 보내야되는 패킷이 많은것
	DWORD _dwSendRef = 0;

	// recv
	OVERLAPPED_EX _RecvRequest;
	CCircleBuffer _RecvDataBuffer; // recv 받으면 여기에 쌓습니다. 오버되면 연결을 터트립니다.
	DWORD _dwRecvRef = 0;

	// inner
	OVERLAPPED_EX _InnerRequest;
	Lock _InnerQueueLock;
	queue<CNetworkBuffer*> _InnerQueue = {}; //
	DWORD _dwInnerRef = 0;

	//
	INT64 _biUpdateTimer = 0;
	INT64 _biHeartbeatTimer = 0;

	//
public:
	CConnector(DWORD dwUniqueIndex = 0);
	virtual ~CConnector();

	bool Initialize();
	bool Finalize();

	DWORD GetUniqueIndex() { return _dwUniqueIndex; }
	DWORD GetIndex() { return _dwUniqueIndex; }
	
	//void SetActive() { InterlockedExchange(&_dwActive, 1); }
	//void SetDeactive() { InterlockedExchange(&_dwActive, 0); }
	//DWORD GetActive() { return InterlockedExchange(&_dwActive, _dwActive); }

	void* SetParam(void *pParam) { return _pParam = pParam; }
	void* GetParam() { return _pParam; }
	bool GetUsed() { return (nullptr != _pParam); }

	SOCKET SetSocket(SOCKET socket) { return _Socket = socket; }
	SOCKET GetSocket() { return _Socket; }
	bool GetActive() { return (INVALID_SOCKET != _Socket); }

	void SetDomain(WCHAR *pwcsDomain, WORD wPort);
	void GetSocket2IP(char *pszIP);
	void ConvertSocket2IP();

	char* GetDomainA() { return _szDomain; }
	WCHAR* GetDomain() { return _wcsDomain; }
	WORD GetPort() { return _wPort; }

	// send
	eResultCode AddSendData(char *pSendData, DWORD dwSendDataSize);
	int AddSendQueue(char *pSendData, DWORD dwSendDataSize); // ret: sendqueue.size
	int SendPrepare(); //ret: send data size
	int SendComplete(DWORD dwSendSize); // ret: remain send data size

	WSABUF* GetSendWSABuffer();
	OVERLAPPED* GetSendOverlapped() { return (OVERLAPPED*)&_SendRequest; }
	DWORD IncSendRef() { InterlockedIncrement(&_dwSendRef); return _dwSendRef; }
	DWORD DecSendRef() { InterlockedDecrement(&_dwSendRef); return _dwSendRef; }
	DWORD GetSendRef() { return _dwSendRef; }

	// recv
	int RecvPrepare(); //ret: recv buffer size
	int RecvComplete(DWORD dwRecvSize); // ret: recvdatabuffer.getdatasize
	virtual int DataParsing();

	WSABUF* GetRecvWSABuffer();
	OVERLAPPED* GetRecvOverlapped() { return (OVERLAPPED*)&_RecvRequest; }
	DWORD IncRecvRef() { InterlockedIncrement(&_dwRecvRef); return _dwRecvRef; }
	DWORD DecRecvRef() { InterlockedDecrement(&_dwRecvRef); return _dwRecvRef; }
	DWORD GetRecvRef() { return _dwRecvRef; }

	// inner
	int AddInnerQueue(char *pSendData, DWORD dwSendDataSize);
	int InnerPrepare();
	int InnerComplete(DWORD dwInnerSize);

	WSABUF* GetInnerWSABuffer();
	OVERLAPPED* GetInnerOverlapped() { return (OVERLAPPED*)&_InnerRequest; }
	DWORD IncInnerRef() { InterlockedIncrement(&_dwInnerRef); return _dwInnerRef; }
	DWORD DecInnerRef() { InterlockedDecrement(&_dwInnerRef); return _dwInnerRef; }
	DWORD GetInnerRef() { return _dwInnerRef; }

	//
	bool CheckUpdateTimer(INT64 biCurrTime) { return (_biUpdateTimer < biCurrTime); }
	bool DoUpdate(INT64 biCurrTime);
	bool CheckHeartbeat(INT64 biCurrTime);
	wstring GetStateReport();
};

//
class CConnectorMgr 
{
private:
	DWORD _dwConnectorIndex = 0;
	DWORD _dwMaxConnectorCount = 0;

	list<CConnector> _ConnectorList = {}; // 생성된 전체 리스트
	list<CConnector*> _FreeConnectorList = {}; // 사용할수있는

public:
	list<CConnector*> _UsedConnectorList = {}; // 사용중인
	Lock _Lock;

	//
private:
	CConnectorMgr(int nConnectorMax = 2000)
	{
		for( int cnt = 0; cnt < nConnectorMax; ++cnt )
		{
			CConnector *pData = new CConnector(GetUniqueIndex());
			
			_ConnectorList.push_back(*pData);
			_FreeConnectorList.push_back(pData);
		}

		_dwMaxConnectorCount = nConnectorMax;
	}

	~CConnectorMgr(void)
	{
		_UsedConnectorList.clear();
		_FreeConnectorList.clear();
		_ConnectorList.clear();
	}

public:
	static CConnectorMgr& GetInstance()
	{
		static CConnectorMgr *pInstance = new CConnectorMgr();
		return *pInstance;
	}
	
	DWORD GetUniqueIndex() 
	{
		InterlockedIncrement((DWORD*)&_dwConnectorIndex); 
		return _dwConnectorIndex; 
	}

	CConnector* GetFreeConnector()
	{
		CConnector *pConnector = nullptr;

		{
			ScopeLock lock(_Lock);

			if( _FreeConnectorList.size() )
			{
				pConnector = _FreeConnectorList.front();
				_FreeConnectorList.pop_front();

				_UsedConnectorList.push_back(pConnector);
			}
		}

		return pConnector;
	}

	void ReleaseConnector(CConnector *pConnector)
	{
		{
			ScopeLock lock(_Lock);

			//pConnector->SetDeactive();

			if( std::find(_UsedConnectorList.begin(), _UsedConnectorList.end(), pConnector) != _UsedConnectorList.end() )
			{
				_UsedConnectorList.remove(pConnector);
				_FreeConnectorList.push_back(pConnector);
			}
		}
	}

	wstring GetStateReport()
	{
		wstring wstrState = {};

		{
			ScopeLock lock(_Lock);
			wstrState.append(FormatW(L"pool:%d, used:%d, free:%d", _ConnectorList.size(), _UsedConnectorList.size(), _FreeConnectorList.size()));
		}

		return wstrState;
	}
};

//
#endif //__CONNECTOR_H__