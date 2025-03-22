#include "stdafx.h"

#include "../_framework/connector.h"
#include "../_lib/util.h"
#include "./packet_svr.h"

//
int Connector::DataParser()
{
    int nPacketLength = 0;

    eResultCode result = ParseNetworkData(_RecvDataBuffer, (DWORD&)nPacketLength);
    switch (result) {
    case eResultCode::invalid_packet:
        return -1;
        break;
    }

    return nPacketLength;
}

bool Connector::DoUpdate(INT64 biCurrTime)
{
    if (0 == _biUpdateTimer) {
        _biUpdateTimer = biCurrTime + MILLISEC_A_MIN;
    }

    //
    if (_biUpdateTimer < biCurrTime) {
        _biUpdateTimer = biCurrTime + MILLISEC_A_MIN;
        g_PerformanceLog.Write(GetReport());

        return true;
    } else {
        return false;
    }
}

bool Connector::CheckHeartbeat(INT64 biCurrTime)
{
    if (0 == _biHeartbeatTimer) {
        _biHeartbeatTimer = biCurrTime + (MILLISEC_A_SEC * 30);
    }

    //
    if (GetActive() && _biHeartbeatTimer < biCurrTime) {
        _biHeartbeatTimer = biCurrTime + (MILLISEC_A_SEC * 30);

        CS_P_HEARTBEAT sendPacket;
        AddSendData((char*)&sendPacket, sizeof(sendPacket));

        return true;
    } else {
        return false;
    }
}

//wstring Connector::GetReport()
//{
//    wstring wstrReport{};
//
//    if (GetActive()) {
//        wstrReport.append(FormatW(L"[%d] connected: %d,", GetUID(), GetSocket()));
//    } else {
//        wstrReport.append(FormatW(L"[%d] disconnected: ", GetUID()));
//    }
//
//    wstrReport.append(FormatW(L"sq:%d,", _SendQueue.size()));
//    wstrReport.append(FormatW(L"rb:(%d/%d),", _RecvDataBuffer.GetDataSize(), _RecvDataBuffer.GetBufferSize()));
//    //wstrReport.append(FormatW(L"iq:%d", _InnerQueue.size()));
//
//    return wstrReport;
//}