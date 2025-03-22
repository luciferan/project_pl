#include "./packet_cli.h"

#include "../_framework/connector.h"
#include "../_lib/util.h"

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