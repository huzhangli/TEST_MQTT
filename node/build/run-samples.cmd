@REM Copyright (c) Microsoft. All rights reserved.
@REM Licensed under the MIT license. See LICENSE file in the project root for full license information.

@setlocal EnableDelayedExpansion
@echo off

set node-root=%~dp0..
rem // resolve to fully qualified path
for %%i in ("%node-root%") do set node-root=%%~fi

set device-samples=%node-root%\device\samples
set service-samples=%node-root%\service\samples
set iothub-explorer=%node-root%\..\tools\iothub-explorer
set node-build-tools=%node-root%\build\tools
set temp-devices-file=%node-root%\devices.txt

set DEVICE_ID=node-samples-tests-%RANDOM%
node %iothub-explorer%\iothub-explorer.js %IOTHUB_CONNECTION_STRING% create %DEVICE_ID%
if errorlevel 1 goto :eof

for /f "usebackq tokens=*" %%i in (`node !iothub-explorer!\iothub-explorer.js !IOTHUB_CONNECTION_STRING! sas-token !DEVICE_ID! ^| find "SharedAccessSignature"`) do set DEVICE_SAS=%%i
if errorlevel 1 goto :deleteDevice

echo Device sends 4 messages to the service (1 per protocol)
node %device-samples%\send_message.js --sas "%DEVICE_SAS%" --amqp
if errorlevel 1 goto :deleteDevice
node %device-samples%\send_message.js --sas "%DEVICE_SAS%" --mqtt
if errorlevel 1 goto :deleteDevice
node %device-samples%\send_message.js --sas "%DEVICE_SAS%" --http
if errorlevel 1 goto :deleteDevice
node %device-samples%\send_message.js --sas "%DEVICE_SAS%" --amqpws
if errorlevel 1 goto :deleteDevice

echo Device sends batched messages to the service
node %device-samples%\send_batch_http.js --sas "%DEVICE_SAS%"
if errorlevel 1 goto :deleteDevice

echo Service sends messages to the device and it's received using AMQP
node %service-samples%\send_c2d_message.js --connectionString %IOTHUB_CONNECTION_STRING% --to %DEVICE_ID% --msg "Hello, AMQP World!"
if errorlevel 1 goto :deleteDevice
node %device-samples%\receive_message.js --sas "%DEVICE_SAS%" --amqp
if errorlevel 1 goto :deleteDevice

echo Service sends messages to the device and it's received using AMQP/WS
node %service-samples%\send_c2d_message.js --connectionString %IOTHUB_CONNECTION_STRING% --to %DEVICE_ID% --msg "Hello, AMQP/WS World!"
if errorlevel 1 goto :deleteDevice
node %device-samples%\receive_message.js --sas "%DEVICE_SAS%" --amqpws
if errorlevel 1 goto :deleteDevice

echo Service sends messages to the device and it's received using MQTT
node %node-build-tools%\create_mqtt_session.js --sas "%DEVICE_SAS%"
if errorlevel 1 goto :deleteDevice
node %service-samples%\send_c2d_message.js --connectionString %IOTHUB_CONNECTION_STRING% --to %DEVICE_ID% --msg "Hello, MQTT World!"
if errorlevel 1 goto :deleteDevice
node %device-samples%\receive_message.js --sas "%DEVICE_SAS%" --mqtt
if errorlevel 1 goto :deleteDevice

echo Service sends messages to the device and it's received using HTTP
node %service-samples%\send_c2d_message.js --connectionString %IOTHUB_CONNECTION_STRING% --to %DEVICE_ID% --msg "Hello, HTTP World!"
if errorlevel 1 goto :deleteDevice
node %device-samples%\receive_message.js --sas "%DEVICE_SAS%" --http
if errorlevel 1 goto :deleteDevice

echo Device uploads a file to a blob and the service is notified of the file upload
node %device-samples%\file_upload.js --sas "%DEVICE_SAS%" --filePath "%node-root%\..\README.md" --blobName "node-samples-test.txt"
if errorlevel 1 goto :deleteDevice
node %service-samples%\receive_file_notifications.js --connectionString %IOTHUB_CONNECTION_STRING%
if errorlevel 1 goto :deleteDevice

echo Test basic registry operations
node %service-samples%\registry_sample.js --connectionString %IOTHUB_CONNECTION_STRING%
if errorlevel 1 goto :deleteDevice

echo Test bulk registry operations
node %service-samples%\registry_bulk_sample.js --import --connectionString %IOTHUB_CONNECTION_STRING% --storageConnectionString %STORAGE_CONNECTION_STRING%
if errorlevel 1 goto :deleteDevice
node %service-samples%\registry_bulk_sample.js --export --outFile %temp-devices-file% --connectionString %IOTHUB_CONNECTION_STRING% --storageConnectionString %STORAGE_CONNECTION_STRING%

:deleteDevice
node %iothub-explorer%\iothub-explorer.js %IOTHUB_CONNECTION_STRING% delete %DEVICE_ID%
if exist %temp-devices-file% del %temp-devices-file%

@endlocal