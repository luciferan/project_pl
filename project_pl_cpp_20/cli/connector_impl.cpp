#include "../_framework/connector.h"
#include "./packet_cli.h"
#include "../_lib/util.h"

//
int Connector::DataParser()
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

bool Connector::DoUpdate(INT64 biCurrTime)
{
    if (_biUpdateTimer < biCurrTime) {
        _biUpdateTimer = biCurrTime + MILLISEC_A_SEC;
        return true;
    } else {
        return false;
    }
}

bool Connector::CheckHeartbeat(INT64 biCurrTime)
{
    if (GetActive() && _biHeartbeatTimer < biCurrTime) {
        _biHeartbeatTimer = biCurrTime + (MILLISEC_A_SEC * 30);
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