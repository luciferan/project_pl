#include "Connector.h"
#include "./packetDataQueue.h"
#include "./network.h"

#include "./util_Time.h"
#include "./util_String.h"
#include "./log.h"

//
CConnector::CConnector(DWORD dwUniqueIndex)
{
	_dwUniqueIndex = dwUniqueIndex;
}

CConnector::~CConnector()
{
	Finalize();
}

bool CConnector::Initialize()
{
	return true;
}

bool CConnector::Finalize()
{
	if( INVALID_SOCKET != _Socket )
		closesocket(_Socket);

	SAFE_DELETE(_RecvRequest.pBuffer);
	SAFE_DELETE(_SendRequest.pBuffer);
	SAFE_DELETE(_InnerRequest.pBuffer);

	return true;
}

void CConnector::SetDomain(WCHAR *pwcsDomain, WORD wPort)
{
	wcsncpy_s(_wcsDomain, eNetwork::MAX_LEN_DOMAIN_STRING, pwcsDomain, wcslen(pwcsDomain));
	_wPort = wPort;

	size_t converted = 0;
	wcstombs_s(&converted, _szDomain, eNetwork::MAX_LEN_DOMAIN_STRING, _wcsDomain, eNetwork::MAX_LEN_DOMAIN_STRING);

	return;
}

void CConnector::GetSocket2IP(char *pszIP)
{
	SOCKADDR_IN SockAddr;
	int iLen = sizeof(SockAddr);

	getpeername(_Socket, (sockaddr*)&SockAddr, &iLen);
	//strncpy_s(pszIP, eSession::MAX_LEN_IP4_STRING, inet_ntoa(SockAddr.sin_addr), iLen);
	inet_ntop(AF_INET, &SockAddr.sin_addr, pszIP, eNetwork::MAX_LEN_IP4_STRING);
}

void CConnector::ConvertSocket2IP()
{
	SOCKADDR_IN SockAddr;
	int iLen = sizeof(SockAddr);

	getpeername(_Socket, (sockaddr*)&SockAddr, &iLen);
	//strncpy_s(_szDomain, eSession::MAX_LEN_IP4_STRING, inet_ntoa(SockAddr.sin_addr), iLen);
	inet_ntop(AF_INET, &SockAddr.sin_addr, _szDomain, _countof(_szDomain));

	size_t convert = 0;
	mbstowcs_s(&convert, _wcsDomain, eNetwork::MAX_LEN_IP4_STRING, _szDomain, eNetwork::MAX_LEN_IP4_STRING);
}

//
eResultCode CConnector::AddSendData(char *pSendData, DWORD dwSendDataSize)
{
	return Network::GetInstance().Write(this, pSendData, dwSendDataSize);
}

int CConnector::AddSendQueue(char *pSendData, DWORD dwSendDataSize)
{
	CNetworkBuffer *pSendBuffer = new CNetworkBuffer;
	if( !pSendBuffer )
		return -1;

	int nRet = pSendBuffer->SetSendData(this, pSendData, dwSendDataSize);
	if( 0 > nRet )
	{
		SAFE_DELETE(pSendBuffer);
		return -1;
	}

	//
	ScopeLock lock(_SendQueueLock);
	_SendQueue.push(pSendBuffer);

	return (int)_SendQueue.size();
}

int CConnector::SendPrepare()
{
	if( _SendQueue.empty() )
		return 0;
	if( 0 < _dwSendRef ) 
		return 0;

	ScopeLock lock(_SendQueueLock);

	CNetworkBuffer *pSendData = _SendQueue.front();
	_SendQueue.pop();

	_SendRequest.ResetOverlapped();
	_SendRequest.pSession = this;
	_SendRequest.pBuffer = pSendData;

	IncSendRef();
	
	return pSendData->GetDataSize();
}

int CConnector::SendComplete(DWORD dwSendSize)
{
	CNetworkBuffer *pBuffer = _SendRequest.pBuffer;
	if( !pBuffer )
		return 0;

	if( dwSendSize < _SendRequest.pBuffer->GetDataSize() )
	{
		pBuffer->_nDataSize -= dwSendSize;
		pBuffer->_WSABuffer.buf += dwSendSize;
		pBuffer->_WSABuffer.len -= dwSendSize;

		return pBuffer->_nDataSize;
	}
	else
	{
		DecSendRef();

		SAFE_DELETE(pBuffer);
		_SendRequest.pBuffer = nullptr;
		_SendRequest.ResetOverlapped();

		return 0;
	}
}

WSABUF* CConnector::GetSendWSABuffer()
{
	//if( _SendRequest.pBuffer )
	return &_SendRequest.pBuffer->_WSABuffer;
	//return nullptr;
}

int CConnector::RecvPrepare()
{
	if( !_RecvRequest.pBuffer )
	{
		CNetworkBuffer *pBuffer = new CNetworkBuffer;
		if( !pBuffer )
			return 0;

		_RecvRequest.ResetOverlapped();
		_RecvRequest.pSession = this;
		_RecvRequest.pBuffer = pBuffer;
	}
	else
	{
		_RecvRequest.ResetOverlapped();
		_RecvRequest.pBuffer->Reset();
	}

	int nRet = _RecvRequest.pBuffer->SetRecvData(this);
	if( nRet )
		InterlockedIncrement(&_dwRecvRef);

	return nRet;
}

int CConnector::RecvComplete(DWORD dwRecvSize)
{
	char *pRecvData = _RecvRequest.pBuffer->_pBuffer;
	_RecvRequest.pBuffer->_nDataSize = dwRecvSize;

	InterlockedDecrement(&_dwRecvRef);

	//
	ULONG nEmptySize = _RecvDataBuffer.GetEmptySize();
	if( nEmptySize < dwRecvSize )
		return -1;

	_RecvDataBuffer.Write(pRecvData, dwRecvSize);

	//
	int nPacketLength = DataParsing();
	if( 0 < nPacketLength )
	{
		CPacketStruct *pPacket = CRecvPacketQueue::GetInstance().GetFreePacketStruct();
		if( !pPacket )
			return -1;

		pPacket->pSession = this;
		pPacket->_nDataSize = _RecvDataBuffer.Read(pPacket->_pBuffer, nPacketLength);
		_RecvDataBuffer.Erase(nPacketLength);

		//
		CRecvPacketQueue::GetInstance().Push(pPacket);
	}
	else
	{
		return -1;
	}

	return _RecvDataBuffer.GetDataSize();
}

WSABUF* CConnector::GetRecvWSABuffer()
{
	//if( _RecvRequest.pBuffer )
	return &_RecvRequest.pBuffer->_WSABuffer;
	//return nullptr;
}

int CConnector::AddInnerQueue(char *pSendData, DWORD dwSendDataSize)
{
	if( !pSendData )
		return -1;
	if( MAX_PACKET_BUFFER_SIZE < dwSendDataSize )
		return -1;

	CNetworkBuffer *pBuffer = new CNetworkBuffer;
	if( !pBuffer )
		return -1;
	int nRet = pBuffer->SetInnerData(this, pSendData, dwSendDataSize);
	if( 0 > nRet )
	{
		SAFE_DELETE(pBuffer);
		return -1;
	}

	//
	ScopeLock lock(_InnerQueueLock);
	_InnerQueue.push(pBuffer);

	return (int)_InnerQueue.size();
}

int CConnector::InnerPrepare()
{
	if( _InnerQueue.empty() )
		return 0;
	if( 0 < _dwInnerRef )
		return 0;

	ScopeLock lock(_InnerQueueLock);

	CNetworkBuffer *pPacket = _InnerQueue.front();
	_InnerQueue.pop();

	_InnerRequest.ResetOverlapped();
	_InnerRequest.pBuffer = pPacket;

	return pPacket->GetDataSize();
}

int CConnector::InnerComplete(DWORD dwInnerSize)
{
	CNetworkBuffer *pBuffer = _InnerRequest.pBuffer;
	if( !pBuffer )
		return -1;
	CPacketStruct *pPacket = (CPacketStruct*)pBuffer;

	//
	CRecvPacketQueue::GetInstance().Push(pPacket);

	return 0;
}

WSABUF* CConnector::GetInnerWSABuffer()
{
	//if( _InnerRequest.pBuffer )
	return &_InnerRequest.pBuffer->_WSABuffer;
	//return nullptr;
}
