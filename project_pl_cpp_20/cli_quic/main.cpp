#include "../_external/msquic/include/msquic/msquic.h"
#include <iostream>

const QUIC_API_TABLE* MsQuic;
HQUIC Registration = nullptr;
HQUIC Configuration = nullptr;
HQUIC Connection = nullptr;

const uint16_t UdpPort = 4567;
const uint32_t SendBufferLength = 100;

using namespace std;

QUIC_STATUS QUIC_API StreamCallback(HQUIC stream, void* /*context*/, QUIC_STREAM_EVENT* event)
{
    switch (event->Type) {
    case QUIC_STREAM_EVENT_SEND_COMPLETE:
        free(event->SEND_COMPLETE.ClientContext);
        cout << "data send " << showbase << hex << stream<< endl;
        break;
    case QUIC_STREAM_EVENT_RECEIVE:
        cout << "data received " << showbase << hex << stream << endl;
        break;
    case QUIC_STREAM_EVENT_PEER_SEND_ABORTED:
        cout << "peer aborted " << showbase << hex << stream << endl;
        break;
    case QUIC_STREAM_EVENT_PEER_SEND_SHUTDOWN:
        cout << "peer shutdown " << showbase << hex << stream << endl;
        break;
    case QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE:
        cout << "all done " << showbase << hex << stream << endl;
        if (!event->SHUTDOWN_COMPLETE.AppCloseInProgress) {
            MsQuic->StreamClose(stream);
        }
        break;
    default:
        break;
    }
    return QUIC_STATUS_SUCCESS;
}

void Send(HQUIC connection)
{
    QUIC_STATUS status;
    HQUIC stream = nullptr;
    uint8_t* sendBufferRaw;
    QUIC_BUFFER* sendBuffer;

    if (QUIC_FAILED(status = MsQuic->StreamOpen(connection, QUIC_STREAM_OPEN_FLAG_NONE, StreamCallback, nullptr, &stream))) {
        cout << "StreamOpen failed. status: " << status << endl;
        goto Error;
    }

    cout << "starting..." << showbase << hex << stream << endl;

    if (QUIC_FAILED(status = MsQuic->StreamStart(stream, QUIC_STREAM_START_FLAG_NONE))) {
        cout << "StreamStart failed. status: " << status << endl;
        MsQuic->StreamClose(stream);
        goto Error;
    }

    sendBufferRaw = (uint8_t*)malloc(sizeof(QUIC_BUFFER) + SendBufferLength);
    if (sendBufferRaw == NULL) {
        cout << "sendBuffer allocation failed" << endl;
        status = QUIC_STATUS_OUT_OF_MEMORY;
        goto Error;
    }

    sendBuffer = (QUIC_BUFFER*)sendBufferRaw;
    sendBuffer->Buffer = sendBufferRaw + sizeof(QUIC_BUFFER);
    sendBuffer->Length = SendBufferLength;

    cout << "sending data..." << showbase << hex << stream << endl;

    if (QUIC_FAILED(status = MsQuic->StreamSend(stream, sendBuffer, 1, QUIC_SEND_FLAG_FIN, sendBuffer))) {
        cout << "StreamSend failed. status: " << status << endl;
        free(sendBufferRaw);
    }

Error:
    if (QUIC_FAILED(status)) {
        MsQuic->ConnectionShutdown(connection, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);
    }
}

QUIC_STATUS QUIC_API ConnectionCallback(HQUIC connection, void* /*context*/, QUIC_CONNECTION_EVENT* event)
{
    switch (event->Type) {
    case QUIC_CONNECTION_EVENT_CONNECTED:
        cout << "connected" << showbase << hex << (void*)connection << endl;
        Send(connection);
        break;
    case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_TRANSPORT:
        if (event->SHUTDOWN_INITIATED_BY_TRANSPORT.Status == QUIC_STATUS_CONNECTION_IDLE) {
            cout << "shutdown on idle " << showbase << hex << (void*)connection << endl;
        } else {
            cout << "shutdown by transport " << showbase << hex << (void*)connection << " : " << event->SHUTDOWN_INITIATED_BY_TRANSPORT.Status << endl;
        }
        break;
    case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_PEER:
        cout << "showdown by peer " << showbase << hex << (void*)connection << " : " << event->SHUTDOWN_INITIATED_BY_PEER.ErrorCode << endl;
        break;
    case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE:
        cout << "all done " << showbase << hex << (void*)connection << endl;
        if (!event->SHUTDOWN_COMPLETE.AppCloseInProgress) {
            MsQuic->ConnectionClose(connection);
        }
        break;
    case QUIC_CONNECTION_EVENT_RESUMPTION_TICKET_RECEIVED:
        cout << "resumption ticket received" << endl;
        break;
    case QUIC_CONNECTION_EVENT_IDEAL_PROCESSOR_CHANGED:
        cout << "ideal processsor " << event->IDEAL_PROCESSOR_CHANGED.IdealProcessor << ", partition index " << event->IDEAL_PROCESSOR_CHANGED.PartitionIndex << endl;
        break;
    default:
        break;

    }

    return QUIC_STATUS_SUCCESS;
}

int main(int argc, char* argv[])
{
    QUIC_STATUS status = QUIC_STATUS_SUCCESS;

    if (QUIC_FAILED(status = MsQuicOpen2(&MsQuic))) {
        cout << "MsQuicOpen2 fail " << status << endl;
        return -1;
    }

    QUIC_REGISTRATION_CONFIG RegConfig = {"quic_client", QUIC_EXECUTION_PROFILE_LOW_LATENCY};
    if (QUIC_FAILED(status = MsQuic->RegistrationOpen(&RegConfig, &Registration))) {
        cout << "RegistrationOpen fail " << status << endl;
        return -1;
    }

    QUIC_BUFFER alpn = {sizeof("sample") - 1, (uint8_t*)"sample"};
    QUIC_SETTINGS settings = {0};

    QUIC_CREDENTIAL_CONFIG credConfig = {};
    credConfig.Type = QUIC_CREDENTIAL_TYPE_NONE;
    credConfig.Flags = QUIC_CREDENTIAL_FLAG_CLIENT | QUIC_CREDENTIAL_FLAG_NO_CERTIFICATE_VALIDATION;

    if (QUIC_FAILED(status = MsQuic->ConfigurationOpen(Registration, &alpn, 1, &settings, sizeof(settings), nullptr, &Configuration))) {
        cout << "ConfigurationOpen failed" << endl;
        return -1;
    }
    if (QUIC_FAILED(status = MsQuic->ConfigurationLoadCredential(Configuration, &credConfig))) {
        cout << "ConfigurationLoadCredential" << endl;
        return -1;
    }

    std::cout << "press any key" << std::endl;
    getchar();
    std::cout << "Connecting to server..." << std::endl;

    //
    if (QUIC_FAILED(status = MsQuic->ConnectionOpen(Registration, ConnectionCallback, nullptr, &Connection))) {
        cout << "ConnectionOpen fail " << status << endl;
        return -1;
    }
    if (QUIC_FAILED(status = MsQuic->ConnectionStart(Connection, Configuration, QUIC_ADDRESS_FAMILY_UNSPEC, "127.0.0.1", UdpPort))) {
        cout << "ConnectionStart fail " << status << endl;
        return -1;
    }

    getchar();

    if(Connection) MsQuic->ConnectionClose(Connection);
    if(Configuration) MsQuic->ConfigurationClose(Configuration);
    if(Registration) MsQuic->RegistrationClose(Registration);
    if(MsQuic) MsQuicClose(MsQuic);

    return 0;
}