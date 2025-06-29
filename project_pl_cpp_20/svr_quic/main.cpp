
#include "../_external/msquic/include/msquic/msquic.h"
#include <iostream>

const QUIC_API_TABLE* MsQuic;
HQUIC Registration = nullptr;
HQUIC Configuration = nullptr;
HQUIC Listener = nullptr;

const uint16_t UdpPort = 4567;
const uint32_t SendBufferLength = 100;

using namespace std;

void Send(HQUIC stream)
{
    void* sendBufferRaw = malloc(sizeof(QUIC_BUFFER) + SendBufferLength);
    if (sendBufferRaw == NULL) {
        cout << "sendBuffer allocation failed" << endl;
        MsQuic->StreamShutdown(stream, QUIC_STREAM_SHUTDOWN_FLAG_ABORT, 0);
        return;
    }

    QUIC_BUFFER* sendBuffer = (QUIC_BUFFER*)sendBufferRaw;
    sendBuffer->Buffer = (uint8_t*)sendBufferRaw + sizeof(QUIC_BUFFER);
    sendBuffer->Length = SendBufferLength;

    cout << "sending data " << showbase << hex << (void*)stream << endl;
    QUIC_STATUS status;
    if (QUIC_FAILED(status = MsQuic->StreamSend(stream, sendBuffer, 1, QUIC_SEND_FLAG_FIN, sendBuffer))) {
        cout << "StreamSend failed " << showbase << hex << (void*)stream << ", status: " << status << endl;
        free(sendBufferRaw);
        MsQuic->StreamShutdown(stream, QUIC_STREAM_SHUTDOWN_FLAG_ABORT, 0);
    }
}

QUIC_STATUS QUIC_API StreamCallback(HQUIC stream, void* /*context*/, QUIC_STREAM_EVENT* event)
{
    switch (event->Type) {
    case QUIC_STREAM_EVENT_SEND_COMPLETE:
        free(event->SEND_COMPLETE.ClientContext);
        cout << "data sent " << showbase << hex << (void*)stream << endl;
        break;
    case QUIC_STREAM_EVENT_RECEIVE:
        cout << "data received " << showbase << hex << (void*)stream << endl;
        break;
    case QUIC_STREAM_EVENT_PEER_SEND_SHUTDOWN:
        cout << "peer shutdown " << showbase << hex << (void*)stream << endl;
        Send(stream);
        break;
    case QUIC_STREAM_EVENT_PEER_SEND_ABORTED:
        cout << "peer aborted " << showbase << hex << (void*)stream << endl;
        MsQuic->StreamShutdown(stream, QUIC_STREAM_SHUTDOWN_FLAG_ABORT, 0);
        break;
    case QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE:
        cout << "all done " << showbase << hex << (void*)stream << endl;
        MsQuic->StreamClose(stream);
        break;
    default:
        break;
    }

    return QUIC_STATUS_SUCCESS;
}

QUIC_STATUS QUIC_API ConnectionCallback(HQUIC connection, void* /*context*/, QUIC_CONNECTION_EVENT* event)
{
    switch (event->Type) {
    case QUIC_CONNECTION_EVENT_CONNECTED:
        cout << "connected" << showbase << hex << (void*)connection << endl;
        MsQuic->ConnectionSendResumptionTicket(connection, QUIC_SEND_RESUMPTION_FLAG_NONE, 0, nullptr);
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
        MsQuic->ConnectionClose(connection);
        break;
    case QUIC_CONNECTION_EVENT_PEER_STREAM_STARTED:
        cout << "peer started " << event->PEER_STREAM_STARTED.Stream << endl;
        MsQuic->SetCallbackHandler(event->PEER_STREAM_STARTED.Stream, (void*)StreamCallback, nullptr);
        break;
    case QUIC_CONNECTION_EVENT_RESUMED:
        cout << "connection resumed " << showbase << hex << (void*)connection << endl;
        break;
    default:
        break;
    }

    return QUIC_STATUS_SUCCESS;
}

QUIC_STATUS QUIC_API ListenerCallback(HQUIC /*listener*/, void* /*context*/, QUIC_LISTENER_EVENT* event)
{
    QUIC_STATUS status = QUIC_STATUS_NOT_SUPPORTED;
    switch (event->Type) {
    case QUIC_LISTENER_EVENT_NEW_CONNECTION:
        MsQuic->SetCallbackHandler(event->NEW_CONNECTION.Connection, (void*)&ConnectionCallback, nullptr);
        status = MsQuic->ConnectionSetConfiguration(event->NEW_CONNECTION.Connection, Configuration);
        break;
    default:
        break;
    }

    return status;
}

typedef struct QUIC_CREDENTIAL_CONFIG_HELPER
{
    QUIC_CREDENTIAL_CONFIG CredConfig;
    union
    {
        QUIC_CERTIFICATE_HASH CertHash;
        QUIC_CERTIFICATE_HASH_STORE CertHashStore;
        QUIC_CERTIFICATE_FILE CertFile;
        QUIC_CERTIFICATE_FILE_PROTECTED CertFileProtected;
    };
} QUIC_CREDENTIAL_CONFIG_HELPER;

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
    settings.PeerBidiStreamCount = 10;
    settings.IsSet.PeerBidiStreamCount = TRUE;

    QUIC_CREDENTIAL_CONFIG_HELPER config = {};
    config.CredConfig.Flags = QUIC_CREDENTIAL_FLAG_NONE;
    config.CertFile.CertificateFile = "../_external/msquic/server_cert.pem";
    config.CertFile.PrivateKeyFile = "../_external/msquic/server_key.pem";
    config.CredConfig.Type = QUIC_CREDENTIAL_TYPE_CERTIFICATE_FILE;
    config.CredConfig.CertificateFile = &config.CertFile;

    if (QUIC_FAILED(status = MsQuic->ConfigurationOpen(Registration, &alpn, 1, &settings, sizeof(settings), nullptr, &Configuration))) {
        cout << "ConfigurationOpen fail " << status << endl;
        return -1;
    }

    if (QUIC_FAILED(status = MsQuic->ConfigurationLoadCredential(Configuration, &config.CredConfig))) {
        cout << "ConfigurationLoadCredential fail " << status << endl;
        return -1;
    }

    QUIC_ADDR addr = {0};
    QuicAddrSetFamily(&addr, QUIC_ADDRESS_FAMILY_UNSPEC);
    QuicAddrSetPort(&addr, UdpPort);

    if (QUIC_FAILED(status = MsQuic->ListenerOpen(Registration, ListenerCallback, nullptr, &Listener))) {
        cout << "ListenerOpen fail " << status << endl;
        return -1;
    }
    if (QUIC_FAILED(status = MsQuic->ListenerStart(Listener, &alpn, 1, &addr))) {
        cout << "ListenerStart fail " << status << endl;
        return -1;
    }

    cout << "Listening..." << endl;
    getchar();

    if (Listener) MsQuic->ListenerClose(Listener);
    if (Configuration) MsQuic->ConfigurationClose(Configuration);
    if (Registration) MsQuic->RegistrationClose(Registration);
    if (MsQuic) MsQuicClose(MsQuic);

    return 0;
}