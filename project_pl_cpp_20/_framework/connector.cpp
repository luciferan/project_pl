#include "stdafx.h"

#include "./connector.h"
#include "./connector_mgr.h"
#include "./packet_data_queue.h"
#include "./network.h"

#include "../_lib/util_Time.h"
#include "../_lib/util_String.h"
#include "../_lib/log.h"

#include <atomic>

// Connector --------------------------------------------------------------------
void Connector::Release()
{
    if (INVALID_SOCKET != _socket) {
        shutdown(_socket, SD_BOTH);
        closesocket(_socket);
        _socket = INVALID_SOCKET;
    }

    SAFE_DELETE(_RecvRequest.pBuffer);
    SAFE_DELETE(_SendRequest.pBuffer);
}

void Connector::TryRelease()
{
    if (INVALID_SOCKET != _socket) {
        shutdown(_socket, SD_BOTH);
        closesocket(_socket);
        _socket = INVALID_SOCKET;
    }

    if (_param == nullptr) {
        Release();
        ConnectorMgr::GetInstance().SetFreeObject(this);
    }
}

void Connector::SetDomainA(char* pszDomain, WORD wPort)
{
    strncpy_s(_szDomain, NetworkConst::MAX_LEN_DOMAIN_STRING, pszDomain, _TRUNCATE);
    _wPort = wPort;

    size_t converted = 0;
    mbstowcs_s(&converted, _wcsDomain, NetworkConst::MAX_LEN_DOMAIN_STRING, _szDomain, _TRUNCATE);

    return;
}

void Connector::SetDomain(WCHAR* pwcsDomain, WORD wPort)
{
    wcsncpy_s(_wcsDomain, NetworkConst::MAX_LEN_DOMAIN_STRING, pwcsDomain, wcslen(pwcsDomain));
    _wPort = wPort;

    size_t converted = 0;
    wcstombs_s(&converted, _szDomain, NetworkConst::MAX_LEN_DOMAIN_STRING, _wcsDomain, _TRUNCATE);

    return;
}

void Connector::GetSocket2IP(char* pszIP)
{
    SOCKADDR_IN SockAddr;
    int iLen = sizeof(SockAddr);

    getpeername(_socket, (sockaddr*)&SockAddr, &iLen);
    //strncpy_s(pszIP, NetworkConst::MAX_LEN_IP4_STRING, inet_ntoa(SockAddr.sin_addr), iLen);
    inet_ntop(AF_INET, &SockAddr.sin_addr, pszIP, NetworkConst::MAX_LEN_IP4_STRING);
}

void Connector::ConvertSocket2IP()
{
    SOCKADDR_IN SockAddr;
    int iLen = sizeof(SockAddr);

    getpeername(_socket, (sockaddr*)&SockAddr, &iLen);
    //strncpy_s(_szDomain, NetworkConst::MAX_LEN_IP4_STRING, inet_ntoa(SockAddr.sin_addr), iLen);
    inet_ntop(AF_INET, &SockAddr.sin_addr, _szDomain, _countof(_szDomain));

    size_t convert = 0;
    mbstowcs_s(&convert, _wcsDomain, NetworkConst::MAX_LEN_IP4_STRING, _szDomain, NetworkConst::MAX_LEN_IP4_STRING);
}


eResultCode Connector::AddSendData(char* pSendData, DWORD dwSendDataSize)
{
    return Network::GetInstance().Write(this, pSendData, dwSendDataSize);
}

int Connector::AddSendQueue(char* pSendData, DWORD dwSendDataSize)
{
    CNetworkBuffer* pSendBuffer = new CNetworkBuffer;
    if (!pSendBuffer) {
        return -1;
    }

    int nRet = pSendBuffer->SetSendData(this, pSendData, dwSendDataSize);
    if (0 > nRet) {
        SAFE_DELETE(pSendBuffer);
        return -1;
    }

    //
    SafeLock lock(_SendQueueLock);
    _SendQueue.push(pSendBuffer);
    return (int)_SendQueue.size();
}

int Connector::SendPrepare()
{
    if (_SendQueue.empty()) {
        return 0;
    }
    if (0 < _dwSendRef) {
        return 0;
    }

    SafeLock lock(_SendQueueLock);

    CNetworkBuffer* pSendData = _SendQueue.front();
    _SendQueue.pop();

    _SendRequest.ResetOverlapped();
    _SendRequest.pConnector = this;
    _SendRequest.pBuffer = pSendData;

    IncSendRef();

    return pSendData->GetDataSize();
}

int Connector::SendComplete(DWORD dwSendSize)
{
    CNetworkBuffer* pBuffer = _SendRequest.pBuffer;
    if (!pBuffer) {
        return 0;
    }

    if (dwSendSize < _SendRequest.pBuffer->GetDataSize()) {
        pBuffer->_nDataSize -= dwSendSize;
        pBuffer->_WSABuffer.buf += dwSendSize;
        pBuffer->_WSABuffer.len -= dwSendSize;

        return pBuffer->_nDataSize;
    } else {
        DecSendRef();

        SAFE_DELETE(pBuffer);
        _SendRequest.pBuffer = nullptr;
        _SendRequest.ResetOverlapped();

        return 0;
    }
}

WSABUF* Connector::GetSendWSABuffer()
{
    //if( _SendRequest.pBuffer )
    //return nullptr;
    return &_SendRequest.pBuffer->_WSABuffer;
}

int Connector::RecvPrepare()
{
    if (!_RecvRequest.pBuffer) {
        CNetworkBuffer* pBuffer = new CNetworkBuffer;
        if (!pBuffer) {
            return 0;
        }

        _RecvRequest.ResetOverlapped();
        _RecvRequest.pConnector = this;
        _RecvRequest.pBuffer = pBuffer;
    } else {
        _RecvRequest.ResetOverlapped();
        _RecvRequest.pBuffer->Reset();
    }

    int nRet = _RecvRequest.pBuffer->SetRecvData(this);
    if (nRet) {
        InterlockedIncrement(&_dwRecvRef);
    }

    return nRet;
}

int Connector::RecvComplete(DWORD dwRecvSize)
{
    char* pRecvData = _RecvRequest.pBuffer->_pBuffer;
    _RecvRequest.pBuffer->_nDataSize = dwRecvSize;

    InterlockedDecrement(&_dwRecvRef);

    //
    ULONG nEmptySize = _RecvDataBuffer.GetEmptySize();
    if (nEmptySize < dwRecvSize) {
        return -1;
    }

    _RecvDataBuffer.Write(pRecvData, dwRecvSize);

    //
    //int nPacketLength = DataParsing();
    int nPacketLength = DataParser();
    if (0 < nPacketLength) {
        CPacketStruct* pPacket = CRecvPacketQueue::GetInstance().GetFreePacketStruct();
        if (!pPacket) {
            return -1;
        }

        pPacket->pConnector = this;
        pPacket->_nDataSize = _RecvDataBuffer.Read(pPacket->_pBuffer, nPacketLength);
        _RecvDataBuffer.Erase(nPacketLength);

        //
        CRecvPacketQueue::GetInstance().Push(pPacket);
    } else {
        return -1;
    }

    return _RecvDataBuffer.GetDataSize();
}

int Connector::DataParsing()
{
    if (funcDataParser) {
        return funcDataParser(_RecvDataBuffer);
    } else {
        return DefaultDataParser(_RecvDataBuffer);
    }
}

WSABUF* Connector::GetRecvWSABuffer()
{
    return &_RecvRequest.pBuffer->_WSABuffer;
}