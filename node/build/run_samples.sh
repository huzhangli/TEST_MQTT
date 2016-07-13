#!/bin/bash

# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.


node_root=$(cd "$(dirname "$0")/.." && pwd)

device_samples=$node_root/device/samples
service_samples=$node_root/service/samples
iothub_explorer=$node_root/../tools/iothub-explorer
node_build_tools=$node_root/build/tools
temp_device_file=$node_root/devices.txt

device_id="node-samples-tests-$RANDOM"

node $iothub_explorer/iothub-explorer.js $IOTHUB_CONNECTION_STRING create $device_id

function delete_device_and_exit {
  echo Deleting test device...
  node $iothub_explorer/iothub-explorer.js $IOTHUB_CONNECTION_STRING delete $device_id
  if [ -e "$temp_device_file" ] 
  then 
    rm "$temp_device_file" 
  fi
  exit $1
}

device_sas=$(node $iothub_explorer/iothub-explorer.js $IOTHUB_CONNECTION_STRING sas-token $device_id | grep SharedAccessSignature)
[ $? -eq 0 ] || delete_device_and_exit $?


echo Device sends 4 messages to the service
node $device_samples/send_message.js --sas "$device_sas" --amqp
[ $? -eq 0 ] || delete_device_and_exit $?
node $device_samples/send_message.js --sas "$device_sas" --mqtt
[ $? -eq 0 ] || delete_device_and_exit $?
node $device_samples/send_message.js --sas "$device_sas" --http
[ $? -eq 0 ] || delete_device_and_exit $?
node $device_samples/send_message.js --sas "$device_sas" --amqpws
[ $? -eq 0 ] || delete_device_and_exit $?

echo Device sends batched messages to the service
node $device_samples/send_batch_http.js --sas "$device_sas"
[ $? -eq 0 ] || delete_device_and_exit $?

echo Service sends messages to the device and it\'s received using AMQP
node $service_samples/send_c2d_message.js --connectionString $IOTHUB_CONNECTION_STRING --to $device_id --msg "Hello, AMQP World!"
[ $? -eq 0 ] || delete_device_and_exit $?
node $device_samples/receive_message.js --sas "$device_sas" --amqp
[ $? -eq 0 ] || delete_device_and_exit $?

echo Service sends messages to the device and it\'s received using AMQP/WS
node $service_samples/send_c2d_message.js --connectionString $IOTHUB_CONNECTION_STRING --to $device_id --msg "Hello, AMQP/WS World!"
[ $? -eq 0 ] || delete_device_and_exit $?
node $device_samples/receive_message.js --sas "$device_sas" --amqpws
[ $? -eq 0 ] || delete_device_and_exit $?

echo Service sends messages to the device and it\'s received using MQTT
node $node_build_tools/create_mqtt_session.js --sas "$device_sas"
[ $? -eq 0 ] || delete_device_and_exit $?
node $service_samples/send_c2d_message.js --connectionString $IOTHUB_CONNECTION_STRING --to $device_id --msg "Hello, MQTT World!"
[ $? -eq 0 ] || delete_device_and_exit $?
node $device_samples/receive_message.js --sas "$device_sas" --mqtt
[ $? -eq 0 ] || delete_device_and_exit $?

echo Service sends messages to the device and it\'s received using HTTP
node $service_samples/send_c2d_message.js --connectionString $IOTHUB_CONNECTION_STRING --to $device_id --msg "Hello, HTTP World!"
[ $? -eq 0 ] || delete_device_and_exit $?
node $device_samples/receive_message.js --sas "$device_sas" --http
[ $? -eq 0 ] || delete_device_and_exit $?

echo Device uploads a file to a blob and the service is notified of the file upload
node $device_samples/file_upload.js --sas "$device_sas" --filePath "$node_root/../README.md" --blobName "node-samples-test-linux.txt"
[ $? -eq 0 ] || delete_device_and_exit $?
node $service_samples/receive_file_notifications.js --connectionString $IOTHUB_CONNECTION_STRING
[ $? -eq 0 ] || delete_device_and_exit $?

echo Test basic registry operations
node $service_samples/registry_sample.js --connectionString $IOTHUB_CONNECTION_STRING
[ $? -eq 0 ] || delete_device_and_exit $?

echo Test bulk registry operations
node $service_samples/registry_bulk_sample.js --import --connectionString $IOTHUB_CONNECTION_STRING --storageConnectionString $STORAGE_CONNECTION_STRING
[ $? -eq 0 ] || delete_device_and_exit $?
node $service_samples/registry_bulk_sample.js --export --outFile "$temp_device_file" --connectionString $IOTHUB_CONNECTION_STRING --storageConnectionString $STORAGE_CONNECTION_STRING
delete_device_and_exit $?