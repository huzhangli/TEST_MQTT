// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "uamqp_integration.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_uamqp_c/message.h"
#include "azure_uamqp_c/amqpvalue.h"
#include "iothub_message.h"

#ifndef RESULT_OK
#define RESULT_OK 0
#define RESULT_FAILURE 1
#endif

static int addPropertiesTouAMQPMessage(IOTHUB_MESSAGE_HANDLE iothub_message_handle, MESSAGE_HANDLE uamqp_message)
{
	int result = RESULT_OK;
	const char* messageId;
	const char* correlationId;
	PROPERTIES_HANDLE uamqp_message_properties;
	int api_call_result;

	if ((api_call_result = message_get_properties(uamqp_message, &uamqp_message_properties)) != 0)
	{
		LogError("Failed to get properties map from uAMQP message (error code %d).", api_call_result);
		result = __LINE__;
	}
	else if (uamqp_message_properties == NULL &&
		     (uamqp_message_properties = properties_create()) == NULL)
	{
		LogError("Failed to create properties map for uAMQP message (error code %d).", api_call_result);
		result = __LINE__;
	}
	else
	{
		if ((messageId = IoTHubMessage_GetMessageId(iothub_message_handle)) != NULL)
		{
			AMQP_VALUE uamqp_message_id;

			if ((uamqp_message_id = amqpvalue_create_string(messageId)) == NULL)
			{
				LogError("Failed to create an AMQP_VALUE for the messageId property value.");
				result = __LINE__;
			}
			else
			{
				if ((api_call_result = properties_set_message_id(uamqp_message_properties, uamqp_message_id)) != 0)
				{
					LogInfo("Failed to set value of uAMQP message 'message-id' property (%d).", api_call_result);
					result = __LINE__;
				}

				amqpvalue_destroy(uamqp_message_id);
			}
		}

		if ((correlationId = IoTHubMessage_GetCorrelationId(iothub_message_handle)) != NULL)
		{
			AMQP_VALUE uamqp_correlation_id;

			if ((uamqp_correlation_id = amqpvalue_create_string(correlationId)) == NULL)
			{
				LogError("Failed to create an AMQP_VALUE for the messageId property value.");
				result = __LINE__;
			}
			else
			{
				if ((api_call_result = properties_set_correlation_id(uamqp_message_properties, uamqp_correlation_id)) != 0)
				{
					LogInfo("Failed to set value of uAMQP message 'message-id' property (%d).", api_call_result);
					result = __LINE__;
				}

				amqpvalue_destroy(uamqp_correlation_id);
			}
		}

		if ((api_call_result = message_set_properties(uamqp_message, uamqp_message_properties)) != 0)
		{
			LogError("Failed to set properties map on uAMQP message (error code %d).", api_call_result);
			result = __LINE__;
		}
	}

	return result;
}

static int addApplicationPropertiesTouAMQPMessage(IOTHUB_MESSAGE_HANDLE iothub_message_handle, MESSAGE_HANDLE uamqp_message)
{
	int result;
	MAP_HANDLE properties_map;
	const char* const* propertyKeys;
	const char* const* propertyValues;
	size_t propertyCount;

	/* Codes_SRS_IOTHUBTRANSPORTUAMQP_01_007: [The IoTHub message properties shall be obtained by calling IoTHubMessage_Properties.] */
	properties_map = IoTHubMessage_Properties(iothub_message_handle);
	if (properties_map == NULL)
	{
		/* Codes_SRS_IOTHUBTRANSPORTUAMQP_01_014: [If any of the APIs fails while building the property map and setting it on the uAMQP message, IoTHubTransportAMQP_DoWork shall notify the failure by invoking the upper layer message send callback with IOTHUB_CLIENT_CONFIRMATION_ERROR.] */
		LogError("Failed to get property map from IoTHub message.");
		result = __LINE__;
	}
	/* Codes_SRS_IOTHUBTRANSPORTUAMQP_01_015: [The actual keys and values, as well as the number of properties shall be obtained by calling Map_GetInternals on the handle obtained from IoTHubMessage_Properties.] */
	else if (Map_GetInternals(properties_map, &propertyKeys, &propertyValues, &propertyCount) != MAP_OK)
	{
		/* Codes_SRS_IOTHUBTRANSPORTUAMQP_01_014: [If any of the APIs fails while building the property map and setting it on the uAMQP message, IoTHubTransportAMQP_DoWork shall notify the failure by invoking the upper layer message send callback with IOTHUB_CLIENT_CONFIRMATION_ERROR.] */
		LogError("Failed to get the internals of the property map.");
		result = __LINE__;
	}
	else
	{
		/* Codes_SRS_IOTHUBTRANSPORTUAMQP_01_016: [If the number of properties is 0, no uAMQP map shall be created and no application properties shall be set on the uAMQP message.] */
		if (propertyCount != 0)
		{
			size_t i;
			/* Codes_SRS_IOTHUBTRANSPORTUAMQP_01_009: [The uAMQP map shall be created by calling amqpvalue_create_map.] */
			AMQP_VALUE uamqp_map = amqpvalue_create_map();
			if (uamqp_map == NULL)
			{
				/* Codes_SRS_IOTHUBTRANSPORTUAMQP_01_014: [If any of the APIs fails while building the property map and setting it on the uAMQP message, IoTHubTransportAMQP_DoWork shall notify the failure by invoking the upper layer message send callback with IOTHUB_CLIENT_CONFIRMATION_ERROR.] */
				LogError("Failed to create uAMQP map for the properties.");
				result = __LINE__;
			}
			else
			{
				for (i = 0; i < propertyCount; i++)
				{
					/* Codes_SRS_IOTHUBTRANSPORTUAMQP_01_010: [A key uAMQP value shall be created by using amqpvalue_create_string.] */
					AMQP_VALUE map_key_value = amqpvalue_create_string(propertyKeys[i]);
					if (map_key_value == NULL)
					{
						/* Codes_SRS_IOTHUBTRANSPORTUAMQP_01_014: [If any of the APIs fails while building the property map and setting it on the uAMQP message, IoTHubTransportAMQP_DoWork shall notify the failure by invoking the upper layer message send callback with IOTHUB_CLIENT_CONFIRMATION_ERROR.] */
						LogError("Failed to create uAMQP property key value.");
						break;
					}

					/* Codes_SRS_IOTHUBTRANSPORTUAMQP_01_011: [A value uAMQP value shall be created by using amqpvalue_create_string.] */
					AMQP_VALUE map_value_value = amqpvalue_create_string(propertyValues[i]);
					if (map_value_value == NULL)
					{
						amqpvalue_destroy(map_key_value);
						/* Codes_SRS_IOTHUBTRANSPORTUAMQP_01_014: [If any of the APIs fails while building the property map and setting it on the uAMQP message, IoTHubTransportAMQP_DoWork shall notify the failure by invoking the upper layer message send callback with IOTHUB_CLIENT_CONFIRMATION_ERROR.] */
						LogError("Failed to create uAMQP property key value.");
						break;
					}

					/* Codes_SRS_IOTHUBTRANSPORTUAMQP_01_008: [All properties shall be transferred to a uAMQP map.] */
					/* Codes_SRS_IOTHUBTRANSPORTUAMQP_01_012: [The key/value pair for the property shall be set into the uAMQP property map by calling amqpvalue_map_set_value.] */
					if (amqpvalue_set_map_value(uamqp_map, map_key_value, map_value_value) != 0)
					{
						amqpvalue_destroy(map_key_value);
						amqpvalue_destroy(map_value_value);
						/* Codes_SRS_IOTHUBTRANSPORTUAMQP_01_014: [If any of the APIs fails while building the property map and setting it on the uAMQP message, IoTHubTransportAMQP_DoWork shall notify the failure by invoking the upper layer message send callback with IOTHUB_CLIENT_CONFIRMATION_ERROR.] */
						LogError("Failed to create uAMQP property key value.");
						break;
					}

					amqpvalue_destroy(map_key_value);
					amqpvalue_destroy(map_value_value);
				}

				if (i < propertyCount)
				{
					result = __LINE__;
				}
				else
				{
					/* Codes_SRS_IOTHUBTRANSPORTUAMQP_01_013: [After all properties have been filled in the uAMQP map, the uAMQP properties map shall be set on the uAMQP message by calling message_set_application_properties.] */
					if (message_set_application_properties(uamqp_message, uamqp_map) != 0)
					{
						/* Codes_SRS_IOTHUBTRANSPORTUAMQP_01_014: [If any of the APIs fails while building the property map and setting it on the uAMQP message, IoTHubTransportAMQP_DoWork shall notify the failure by invoking the upper layer message send callback with IOTHUB_CLIENT_CONFIRMATION_ERROR.] */
						LogError("Failed to transfer the message properties to the uAMQP message.");
						result = __LINE__;
					}
					else
					{
						result = 0;
					}
				}

				amqpvalue_destroy(uamqp_map);
			}
		}
		else
		{
			result = 0;
		}
	}

	return result;
}

static int readPropertiesFromuAMQPMessage(IOTHUB_MESSAGE_HANDLE iothub_message_handle, MESSAGE_HANDLE uamqp_message)
{
	int return_value;
	PROPERTIES_HANDLE uamqp_message_properties;
	AMQP_VALUE uamqp_message_property;
	const char* uamqp_message_property_value;
	int api_call_result;

	/* Codes_SRS_IOTHUBTRANSPORTAMQP_09_155: [uAMQP message properties shall be retrieved using message_get_properties.] */
	if ((api_call_result = message_get_properties(uamqp_message, &uamqp_message_properties)) != 0)
	{
		/* Codes_SRS_IOTHUBTRANSPORTAMQP_09_156: [If message_get_properties fails, the error shall be notified and ‘on_message_received’ shall continue.] */
		LogError("Failed to get property properties map from uAMQP message (error code %d).", api_call_result);
		return_value = __LINE__;
	}
	else
	{
		return_value = 0; // Properties 'message-id' and 'correlation-id' are optional according to the AMQP 1.0 spec.

		/* Codes_SRS_IOTHUBTRANSPORTAMQP_09_157: [The message-id property shall be read from the uAMQP message by calling properties_get_message_id.] */
		if ((api_call_result = properties_get_message_id(uamqp_message_properties, &uamqp_message_property)) == 0)
		{
			// TODO: clean this from documentation.
			///* Codes_SRS_IOTHUBTRANSPORTAMQP_09_158: [If properties_get_message_id fails, the error shall be notified and ‘on_message_received’ shall continue.] */
			//LogInfo("Failed to get value of uAMQP message 'message-id' property (%d).", api_call_result);
			//return_value = __LINE__;

			if (amqpvalue_get_type(uamqp_message_property) != AMQP_TYPE_NULL)
			{
				/* Codes_SRS_IOTHUBTRANSPORTAMQP_09_159: [The message-id value shall be retrieved from the AMQP_VALUE as char* by calling amqpvalue_get_string.] */
				if ((api_call_result = amqpvalue_get_string(uamqp_message_property, &uamqp_message_property_value)) != 0)
				{
					/* Codes_SRS_IOTHUBTRANSPORTAMQP_09_160: [If amqpvalue_get_string fails, the error shall be notified and ‘on_message_received’ shall continue.] */
					LogError("Failed to get value of uAMQP message 'message-id' property (%d).", api_call_result);
					return_value = __LINE__;
				}
				/* Codes_SRS_IOTHUBTRANSPORTAMQP_09_161: [The message-id property shall be set on the IOTHUB_MESSAGE_HANDLE by calling IoTHubMessage_SetMessageId, passing the value read from the uAMQP message.] */
				else if (IoTHubMessage_SetMessageId(iothub_message_handle, uamqp_message_property_value) != IOTHUB_MESSAGE_OK)
				{
					/* Codes_SRS_IOTHUBTRANSPORTAMQP_09_162: [If IoTHubMessage_SetMessageId fails, the error shall be notified and ‘on_message_received’ shall continue.] */
					LogError("Failed to set IOTHUB_MESSAGE_HANDLE 'message-id' property.");
					return_value = __LINE__;
				}
			}
		}

		/* Codes_SRS_IOTHUBTRANSPORTAMQP_09_163: [The correlation-id property shall be read from the uAMQP message by calling properties_get_correlation_id.] */
		if ((api_call_result = properties_get_correlation_id(uamqp_message_properties, &uamqp_message_property)) == 0)
		{
			//TODO: clean this from documentation.
			///* Codes_SRS_IOTHUBTRANSPORTAMQP_09_164: [If properties_get_correlation_id fails, the error shall be notified and ‘on_message_received’ shall continue.] */
			//LogError("Failed to get value of uAMQP message 'correlation-id' property (%d).", api_call_result);
			//return_value = __LINE__;

			if (amqpvalue_get_type(uamqp_message_property) != AMQP_TYPE_NULL)
			{
				/* Codes_SRS_IOTHUBTRANSPORTAMQP_09_165: [The correlation-id value shall be retrieved from the AMQP_VALUE as char* by calling amqpvalue_get_string.] */
				if ((api_call_result = amqpvalue_get_string(uamqp_message_property, &uamqp_message_property_value)) != 0)
				{
					/* Codes_SRS_IOTHUBTRANSPORTAMQP_09_166: [If amqpvalue_get_string fails, the error shall be notified and ‘on_message_received’ shall continue.] */
					LogError("Failed to get value of uAMQP message 'correlation-id' property (%d).", api_call_result);
					return_value = __LINE__;
				}
				/* Codes_SRS_IOTHUBTRANSPORTAMQP_09_167: [The correlation-id property shall be set on the IOTHUB_MESSAGE_HANDLE by calling IoTHubMessage_SetCorrelationId, passing the value read from the uAMQP message.] */
				else if (IoTHubMessage_SetCorrelationId(iothub_message_handle, uamqp_message_property_value) != IOTHUB_MESSAGE_OK)
				{
					/* Codes_SRS_IOTHUBTRANSPORTAMQP_09_168: [If IoTHubMessage_SetCorrelationId fails, the error shall be notified and ‘on_message_received’ shall continue.] */
					LogError("Failed to set IOTHUB_MESSAGE_HANDLE 'correlation-id' property.");
					return_value = __LINE__;
				}
			}
		}

		properties_destroy(uamqp_message_properties);
	}

	return return_value;
}

static int readApplicationPropertiesFromuAMQPMessage(IOTHUB_MESSAGE_HANDLE iothub_message_handle, MESSAGE_HANDLE uamqp_message)
{
	int result;
	AMQP_VALUE uamqp_app_properties;
	uint32_t property_count;
	MAP_HANDLE iothub_message_properties_map;

	// Codes_SRS_IOTHUBTRANSPORTAMQP_09_170: [The IOTHUB_MESSAGE_HANDLE properties shall be retrieved using IoTHubMessage_Properties.]
	if ((iothub_message_properties_map = IoTHubMessage_Properties(iothub_message_handle)) == NULL)
	{
		// Codes_SRS_IOTHUBTRANSPORTAMQP_09_186: [If IoTHubMessage_Properties fails, the error shall be notified and ‘on_message_received’ shall continue.]
		LogError("Failed to get property map from IoTHub message.");
		result = __LINE__;
	}
	// Codes_SRS_IOTHUBTRANSPORTAMQP_09_171: [uAMQP message application properties shall be retrieved using message_get_application_properties.]
	else if ((result = message_get_application_properties(uamqp_message, &uamqp_app_properties)) != 0)
	{
		// Codes_SRS_IOTHUBTRANSPORTAMQP_09_172: [If message_get_application_properties fails, the error shall be notified and ‘on_message_received’ shall continue.]
		LogError("Failed reading the incoming uAMQP message properties (return code %d).", result);
		result = __LINE__;
	}
	// Codes_SRS_IOTHUBTRANSPORTAMQP_09_187: [If message_get_application_properties succeeds but returns a NULL application properties map (there are no properties), ‘on_message_received’ shall continue normally.]
	else if (uamqp_app_properties == NULL)
	{
		result = 0;
	}
	// Codes_SRS_IOTHUBTRANSPORTAMQP_09_173: [The actual uAMQP message application properties should be extracted from the result of message_get_application_properties using amqpvalue_get_inplace_described_value.]
	else if ((uamqp_app_properties = amqpvalue_get_inplace_described_value(uamqp_app_properties)) == NULL)
	{
		// Codes_SRS_IOTHUBTRANSPORTAMQP_09_174: [If amqpvalue_get_inplace_described_value fails, the error shall be notified and ‘on_message_received’ shall continue.]
		LogError("Failed getting the map of uAMQP message application properties (return code %d).", result);
		result = __LINE__;
	}
	// Codes_SRS_IOTHUBTRANSPORTAMQP_09_175: [The number of items in the uAMQP message application properties shall be obtained using amqpvalue_get_map_pair_count.]
	else if ((result = amqpvalue_get_map_pair_count(uamqp_app_properties, &property_count)) != 0)
	{
		// Codes_SRS_IOTHUBTRANSPORTAMQP_09_176: [If amqpvalue_get_map_pair_count fails, the error shall be notified and ‘on_message_received’ shall continue.]
		LogError("Failed reading the number of values in the uAMQP property map (return code %d).", result);
		result = __LINE__;
	}
	else
	{
		// Codes_SRS_IOTHUBTRANSPORTAMQP_09_177: [‘on_message_received’ shall iterate through each uAMQP application property and add it on IOTHUB_MESSAGE_HANDLE properties.]
		size_t i;
		for (i = 0; i < property_count; i++)
		{
			AMQP_VALUE map_key_name;
			AMQP_VALUE map_key_value;
			const char *key_name;
			const char* key_value;

			// Codes_SRS_IOTHUBTRANSPORTAMQP_09_178: [The uAMQP application property name and value shall be obtained using amqpvalue_get_map_key_value_pair.]
			if ((result = amqpvalue_get_map_key_value_pair(uamqp_app_properties, i, &map_key_name, &map_key_value)) != 0)
			{
				// Codes_SRS_IOTHUBTRANSPORTAMQP_09_179: [If amqpvalue_get_map_key_value_pair fails, the error shall be notified and ‘on_message_received’ shall continue.]
				LogError("Failed reading the key/value pair from the uAMQP property map (return code %d).", result);
				result = __LINE__;
				break;
			}
			// Codes_SRS_IOTHUBTRANSPORTAMQP_09_180: [The uAMQP application property name shall be extracted as string using amqpvalue_get_string.]
			else if ((result = amqpvalue_get_string(map_key_name, &key_name)) != 0)
			{
				// Codes_SRS_IOTHUBTRANSPORTAMQP_09_181: [If amqpvalue_get_string fails, the error shall be notified and ‘on_message_received’ shall continue.]
				LogError("Failed parsing the uAMQP property name (return code %d).", result);
				result = __LINE__;
				break;
			}
			// Codes_SRS_IOTHUBTRANSPORTAMQP_09_182: [The uAMQP application property value shall be extracted as string using amqpvalue_get_string.]
			else if ((result = amqpvalue_get_string(map_key_value, &key_value)) != 0)
			{
				// Codes_SRS_IOTHUBTRANSPORTAMQP_09_183: [If amqpvalue_get_string fails, the error shall be notified and ‘on_message_received’ shall continue.]
				LogError("Failed parsing the uAMQP property value (return code %d).", result);
				result = __LINE__;
				break;
			}
			// Codes_SRS_IOTHUBTRANSPORTAMQP_09_184: [The application property name and value shall be added to IOTHUB_MESSAGE_HANDLE properties using Map_AddOrUpdate.]
			else if (Map_AddOrUpdate(iothub_message_properties_map, key_name, key_value) != MAP_OK)
			{
				// Codes_SRS_IOTHUBTRANSPORTAMQP_09_185: [If Map_AddOrUpdate fails, the error shall be notified and ‘on_message_received’ shall continue.]
				LogError("Failed to add/update IoTHub message property map.");
				result = __LINE__;
				break;
			}
		}
	}

	return result;
}

int IoTHubMessage_CreateFromUamqpMessage(MESSAGE_HANDLE uamqp_message, IOTHUB_MESSAGE_HANDLE* iothubclient_message)
{
	int result = __LINE__;

	IOTHUB_MESSAGE_HANDLE iothub_message = NULL;
	MESSAGE_BODY_TYPE body_type;


	if (message_get_body_type(uamqp_message, &body_type) != 0)
	{
		LogError("Failed to get the type of the uamqp message.");
		result = __LINE__;
	}
	else
	{
		if (body_type == MESSAGE_BODY_TYPE_DATA)
		{
			BINARY_DATA binary_data;
			if (message_get_body_amqp_data(uamqp_message, 0, &binary_data) != 0)
			{
				LogError("Failed to get the body of the uamqp message.");
				result = __LINE__;
			}
			else
			{
				iothub_message = IoTHubMessage_CreateFromByteArray(binary_data.bytes, binary_data.length);
			}
		}
	}

	if (iothub_message != NULL)
	{
		if (readPropertiesFromuAMQPMessage(iothub_message, uamqp_message) != 0)
		{
			LogError("Failed reading properties of the uamqp message.");
			IoTHubMessage_Destroy(iothub_message);
			result = __LINE__;
		}
		else if (readApplicationPropertiesFromuAMQPMessage(iothub_message, uamqp_message) != 0)
		{
			LogError("Failed reading application properties of the uamqp message.");
			IoTHubMessage_Destroy(iothub_message);
			result = __LINE__;
		}
		else
		{
			*iothubclient_message = iothub_message;
			result = RESULT_OK;
		}
	}

	return result;
}

int message_create_from_iothub_message(IOTHUB_MESSAGE_HANDLE iothub_message, MESSAGE_HANDLE* uamqp_message)
{
	int result = __LINE__;

	IOTHUBMESSAGE_CONTENT_TYPE contentType = IoTHubMessage_GetContentType(iothub_message);
	const char* messageContent = NULL;
	size_t messageContentSize = 0;
	MESSAGE_HANDLE uamqp_message_tmp = NULL;

	if (contentType == IOTHUBMESSAGE_BYTEARRAY &&
		IoTHubMessage_GetByteArray(iothub_message, &(const unsigned char *)messageContent, &messageContentSize) != IOTHUB_MESSAGE_OK)
	{
		LogError("Failed getting the BYTE array representation of the IOTHUB_MESSAGE_HANDLE instance.");
		result = __LINE__;
	}
	else if (contentType == IOTHUBMESSAGE_STRING &&
		((messageContent = IoTHubMessage_GetString(iothub_message)) == NULL))
	{
		LogError("Failed getting the STRING representation of the IOTHUB_MESSAGE_HANDLE instance.");
		result = __LINE__;
	}
	else if (contentType == IOTHUBMESSAGE_UNKNOWN)
	{
		LogError("Cannot parse IOTHUB_MESSAGE_HANDLE with content type IOTHUBMESSAGE_UNKNOWN.");
		result = __LINE__;
	}
	else if ((uamqp_message_tmp = message_create()) == NULL)
	{
		LogError("Failed allocating the uAMQP message.");
		result = __LINE__;
	}
	else
	{
		BINARY_DATA binary_data;

		if (contentType == IOTHUBMESSAGE_STRING)
		{
			messageContentSize = strlen(messageContent);
		}

		binary_data.bytes = (const unsigned char *)messageContent;
		binary_data.length = messageContentSize;

		if (message_add_body_amqp_data(uamqp_message_tmp, binary_data) != RESULT_OK)
		{
			LogError("Failed setting the body of the uAMQP message.");
			result = __LINE__;
		}
		else if (addPropertiesTouAMQPMessage(iothub_message, uamqp_message_tmp) != RESULT_OK)
		{
			LogError("Failed setting properties of the uAMQP message.");
			result = __LINE__;
		}
		else if (addApplicationPropertiesTouAMQPMessage(iothub_message, uamqp_message_tmp) != RESULT_OK)
		{
			LogError("Failed setting application properties of the uAMQP message.");
			result = __LINE__;
		}
		else
		{
			*uamqp_message = uamqp_message_tmp;
			result = RESULT_OK;
		}
	}

	return result;
}

