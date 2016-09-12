// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "iothubtransportamqp_websockets.h"
#include "azure_c_shared_utility/wsio.h"
#include "iothubtransportamqp.c"

#define DEFAULT_WS_PROTOCOL_NAME "AMQPWSB10"
#define DEFAULT_WS_RELATIVE_PATH "/$iothub/websocket"
#define DEFAULT_WS_PORT 443

XIO_HANDLE getWebSocketsIOTransport(const char* fqdn, int port)
{
	WSIO_CONFIG ws_io_config;
    XIO_HANDLE result;

    const IO_INTERFACE_DESCRIPTION* tlsio_interface = platform_get_default_tlsio();
    if (tlsio_interface == NULL)
    {
        (void)printf("Error getting tlsio interface description.");
        result = NULL;
    }
    else
    {
        TLSIO_CONFIG tlsio_config;
        XIO_HANDLE tlsio;

        tlsio_config.hostname = "iot-sdks-test.azure-devices.net";
        tlsio_config.port = port;
        tlsio = xio_create(tlsio_interface, &tlsio_config);
        if (tlsio == NULL)
        {
            (void)printf("Error creating tlsio.");
            result = NULL;
        }
        else
        {
            ws_io_config.hostname = fqdn;
            ws_io_config.underlying_io = tlsio;

            result = xio_create(wsio_get_interface_description(), &ws_io_config);
        }
    }

    return result;
}
static TRANSPORT_LL_HANDLE IoTHubTransportAMQP_Create_WebSocketsOverTls(const IOTHUBTRANSPORT_CONFIG* config)
{
	AMQP_TRANSPORT_INSTANCE* transport_state = (AMQP_TRANSPORT_INSTANCE*)IoTHubTransportAMQP_Create(config);

	if (transport_state != NULL)
	{
		transport_state->tls_io_transport_provider = getWebSocketsIOTransport;
		transport_state->iotHubPort = DEFAULT_WS_PORT;
	}

	return transport_state;
}

static TRANSPORT_PROVIDER thisTransportProvider_WebSocketsOverTls = {
	IoTHubTransportAMQP_GetHostname,
	IoTHubTransportAMQP_SetOption,
	IoTHubTransportAMQP_Create_WebSocketsOverTls,
	IoTHubTransportAMQP_Destroy,
	IoTHubTransportAMQP_Register,
	IoTHubTransportAMQP_Unregister,
	IoTHubTransportAMQP_Subscribe,
	IoTHubTransportAMQP_Unsubscribe,
	IoTHubTransportAMQP_DoWork,
	IoTHubTransportAMQP_GetSendStatus
};

extern const TRANSPORT_PROVIDER* AMQP_Protocol_over_WebSocketsTls(void)
{
	return &thisTransportProvider_WebSocketsOverTls;
}
