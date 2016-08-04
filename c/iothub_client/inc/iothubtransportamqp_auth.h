// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef IOTHUBTRANSPORTAMQP_AUTH_H
#define IOTHUBTRANSPORTAMQP_AUTH_H

#include "azure-uamqp-c/cbs.h"
#include "azure_c_shared_utility/strings.h"

#ifdef __cplusplus
extern "C"
{
#endif

	typedef enum CBS_STATE_TAG
	{
		CBS_STATE_IDLE,
		CBS_STATE_AUTH_IN_PROGRESS,
		CBS_STATE_AUTHENTICATED
	} CBS_STATE;

	typedef enum AMQP_TRANSPORT_CREDENTIAL_TYPE_TAG
	{
		CREDENTIAL_NOT_BUILD,
		X509,
		DEVICE_KEY,
		DEVICE_SAS_TOKEN,
	} AMQP_TRANSPORT_CREDENTIAL_TYPE;

	typedef struct X509_CREDENTIAL_TAG
	{
		const char* x509certificate;
		const char* x509privatekey;
	} X509_CREDENTIAL;

	typedef union AMQP_TRANSPORT_CREDENTIAL_UNION_TAG
	{
		// Key associated to the device to be used.
		STRING_HANDLE deviceKey;

		// SAS associated to the device to be used.
		STRING_HANDLE deviceSasToken;

		// X509 
		X509_CREDENTIAL x509credential;
	} AMQP_TRANSPORT_CREDENTIAL_UNION;

	typedef struct AMQP_TRANSPORT_CREDENTIAL_TAG
	{
		AMQP_TRANSPORT_CREDENTIAL_TYPE credentialType;
		AMQP_TRANSPORT_CREDENTIAL_UNION credential;
	} AMQP_TRANSPORT_CREDENTIAL;

	typedef struct CBS_AUTHENTICATION_STATE_TAG
	{
		// @brief Lifetime of the SAS token in seconds, from the time it is created.
		double sas_token_lifetime_secs;
		// @brief Maximum period of time to wait before refreshing the SAS token, in seconds.
		double sas_token_refresh_time_secs;
		// @brief Maximum period of time to wait for CBS response for a SAS token put operation.
		double cbs_authentication_timeout;

		// @brief Time when the current SAS token was created, in seconds since epoch.
		double current_sas_token_create_time;

		// @brief Constains the device id, key and/or SAS token.
		IOTHUB_DEVICE_CONFIG device_config;

		// @brief A component of the SAS token. Currently this must be an empty string.
		STRING_HANDLE sasTokenKeyName;

		// @brief Current state of the CBS connection.
		CBS_STATE cbs_state;

		// @brief Connection instance with the Azure IoT CBS.
		CBS_HANDLE cbs;
	} CBS_AUTHENTICATION_STATE;

	typedef AUTHENTICATION_STATE_HANDLE void*;

	/** @brief 
	*
	*	@details 
	*
	*   @returns 
	*/
	extern AUTHENTICATION_STATE_HANDLE authentication_create(void);

	/** @brief
	*
	*	@details
	*
	*   @returns
	*/
	extern int authentication_authenticate(AUTHENTICATION_STATE_HANDLE authentication_state);

	/** @brief
	*
	*	@details
	*
	*   @returns
	*/
	extern int authentication_get_status(AUTHENTICATION_STATE_HANDLE authentication_state, );

	/** @brief
	*
	*	@details
	*
	*   @returns
	*/
	extern int authentication_refresh(AUTHENTICATION_STATE_HANDLE authentication_state);

	/** @brief
	*
	*	@details
	*
	*   @returns
	*/
	extern int authentication_set_option(AUTHENTICATION_STATE_HANDLE authentication_state, const char* name, const void* value);

	/** @brief
	*
	*	@details
	*
	*   @returns
	*/
	extern int authentication_destroy(AUTHENTICATION_STATE_HANDLE authentication_state);

#ifdef __cplusplus
}
#endif

#endif /*IOTHUBTRANSPORTAMQP_AUTH_H*/

