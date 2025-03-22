#include "stdafx.h"

#include "./connector.h"

#include "../_lib/util.h"

//
int DefaultDataParser(CCircleBuffer &buffer)
{
    int nPacketLength = 0;
    buffer.Read((char*)&nPacketLength, sizeof(int));

    if ((int)buffer.GetDataSize() < nPacketLength) {
        return 0;
    }

    return nPacketLength;
}

//int Connector::DataParsing()
//{
//    int nPacketLength = 0;
//    _RecvDataBuffer.Read((char*)&nPacketLength, sizeof(int));
//
//    if ((int)_RecvDataBuffer.GetDataSize() < nPacketLength) {
//        return 0;
//    }
//
//    return nPacketLength;
//}

//int Connector::DataParsing()
//{
//	int nPacketLength = 0;
//	_RecvDataBuffer.Read((char*)&nPacketLength, sizeof(int));
//	nPacketLength = htonl(nPacketLength);
//
//	if( _RecvDataBuffer.GetDataSize() < nPacketLength )
//		return 0;
//
//	return nPacketLength;
//}

//bool Connector::DoUpdate(INT64 biCurrTime)
//{
//    if (_biUpdateTimer < biCurrTime) {
//        _biUpdateTimer = biCurrTime + MILLISEC_A_SEC;
//        return true;
//    } else {
//        return false;
//    }
//}

//bool Connector::CheckHeartbeat(INT64 biCurrTime)
//{
//    if (GetActive() && _biHeartbeatTimer < biCurrTime) {
//        _biHeartbeatTimer = biCurrTime + (MILLISEC_A_SEC * 30);
//        return true;
//    } else {
//        return false;
//    }
//}

string Connector::GetReportA()
{
    string strReport{};

    if (GetActive()) {
        strReport.append(FormatA("[%d] connected: %d,", GetUID(), GetSocket()));
    } else {
        strReport.append(FormatA("[%d] disconnected: ", GetUID()));
    }

    strReport.append(FormatA("sq:%d,", _SendQueue.size()));
    strReport.append(FormatA("rb:(%d/%d),", _RecvDataBuffer.GetDataSize(), _RecvDataBuffer.GetBufferSize()));

    return strReport;
}

wstring Connector::GetReport()
{
    wstring wstrReport{};

    if (GetActive()) {
        wstrReport.append(FormatW(L"[%d] connected: %d,", GetUID(), GetSocket()));
    } else {
        wstrReport.append(FormatW(L"[%d] disconnected: ", GetUID()));
    }

    wstrReport.append(FormatW(L"sq:%d,", _SendQueue.size()));
    wstrReport.append(FormatW(L"rb:(%d/%d),", _RecvDataBuffer.GetDataSize(), _RecvDataBuffer.GetBufferSize()));

    return wstrReport;
}