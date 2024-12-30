#pragma once
#ifndef __USERSESSION_H__
#define __USERSESSION_H__

//
#include <Windows.h>

#include <list>
#include <queue>

#include "../_framework/safeLock.h"

//
class CConnector;

//
class CUserSession
{
private:
	DWORD _dwIndex = 0;

	CConnector* _pConnector = nullptr;

	DWORD _dwUsed = 0;
	DWORD _dwActive = 0;

	INT64 _uiHeartBeat = 0;

	//
public:
	CUserSession(DWORD dwIndex = 0)
	{
		_dwIndex = dwIndex;
	}
	virtual ~CUserSession()
	{
	}

	int clear();

	DWORD GetIndex() { return _dwIndex; }
	void SetConnector(CConnector *pConnector) { _pConnector = pConnector; }

	//
	int MessageProcess(char *pData, int nLen);

	int DoUpdate(INT64 uiCurrTime);
	eResultCode SendPacketData(DWORD dwProtocol, char *pData, DWORD dwDataSize);

	//
	eResultCode ReqAuth();
	eResultCode ReqEcho();

	eResultCode RepHeartBeat();
};

//
class CUserSessionMgr
{
private:
	DWORD _dwUserSessionIndex;
	int _nMaxUserSessionCount;
	std::list<CUserSession> _UserSessionList;
	std::list<CUserSession*> _FreeUserSessionList;
	
public:
	std::list<CUserSession*> _UsedUserSessionList;
	Lock _Lock;

	//
private:
	CUserSessionMgr(int nMaxCount = 2000)
	{
		for( int cnt = 0; cnt < nMaxCount; ++cnt )
		{
			CUserSession *pData = new CUserSession(GetUniqueIndex());

			_UserSessionList.push_back(*pData);
			_FreeUserSessionList.push_back(pData);
		}

		_nMaxUserSessionCount = nMaxCount;
	}

	~CUserSessionMgr()
	{
		_UserSessionList.clear();
		_FreeUserSessionList.clear();
		_UsedUserSessionList.clear();
	}

	DWORD GetUniqueIndex()
	{
		InterlockedIncrement((DWORD*)&_dwUserSessionIndex);
		return _dwUserSessionIndex;
	}

public:
	static CUserSessionMgr& GetInstance()
	{
		static CUserSessionMgr *pInstance = new CUserSessionMgr();
		return *pInstance;
	}

	CUserSession* GetFreeUserSession()
	{
		CUserSession *pRet = nullptr;

		{
			SafeLock lock(_Lock);

			if( false == _FreeUserSessionList.empty() )
			{
				pRet = _FreeUserSessionList.front();
				_FreeUserSessionList.pop_front();

				_UsedUserSessionList.push_back(pRet);
			}
		}

		return pRet;
	}

	void ReleaseUserSesssion(CUserSession *pUserSession)
	{
		{
			SafeLock lock(_Lock);

			if( std::find(_UsedUserSessionList.begin(), _UsedUserSessionList.end(), pUserSession) != _UsedUserSessionList.end() )
			{
				_UsedUserSessionList.remove(pUserSession);
				_FreeUserSessionList.push_back(pUserSession);
			}
			else
			{
			}
		}
	}
};

#endif //__USERSESSION_H__