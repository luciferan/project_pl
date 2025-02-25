#include "../_framework/Connector.h"
#include "../_framework/packet.h"
#include "../_framework/packet_cli.h"
#include "../_lib/util.h"

#include <string>
#include <format>

//
int CConnector::DataParsing()
{
    int nPacketLength = 0;

    DWORD dwRet = ParseNetworkData(_RecvDataBuffer, (DWORD&)nPacketLength);
    switch (dwRet) {
        case eResultCode::RESULT_INVALID_PACKET:
            return -1;
            break;
    }

    return nPacketLength;
}

bool CConnector::DoUpdate(INT64 biCurrTime)
{
    if (0 == _biUpdateTimer) {
        _biUpdateTimer = biCurrTime + MILLISEC_A_MIN;
    }

    //
    if (_biUpdateTimer < biCurrTime) {
        _biUpdateTimer = biCurrTime + MILLISEC_A_MIN;
        g_PerformanceLog.Write(GetStateReport());

        return true;
    } else {
        return false;
    }
}

bool CConnector::CheckHeartbeat(INT64 biCurrTime)
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

wstring CConnector::GetStateReport()
{
    wstring wstrReport{};

    if (GetActive()) {
        wstrReport.append(FormatW(L"[%d] connected: %d,", GetUID(), GetSocket()));
    } else {
        wstrReport.append(FormatW(L"[%d] disconnected: ", GetUID()));
    }

    wstrReport.append(FormatW(L"sq:%d,", _SendQueue.size()));
    wstrReport.append(FormatW(L"rb:(%d/%d),", _RecvDataBuffer.GetDataSize(), _RecvDataBuffer.GetBufferSize()));
    wstrReport.append(FormatW(L"iq:%d", _InnerQueue.size()));

    return wstrReport;
}