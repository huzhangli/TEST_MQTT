---
platform: debian, fedora, Linux, opensuse, raspbian, Ubuntu, windows, yocto 
device: any
language: javascript
---

Running Node.js samples
===
---

# Table of Contents

-   [Introduction](#Introduction)
-   [Prerequisites](#Prerequisites)
-   [Getting and running the device samples](#DeviceSamples)
-   [Getting and running the service samples](#ServiceSamples)

<a name="Introduction"></a>
# Introduction and samples overview
This document describes how to run the Node.js SDK samples for the Device and Service SDKs.

The device samples are meant to run on a device but can be run from any computer running Node.js 0.10 or above. The samples demonstrate how to:
- Send a message or a batched set of messages as a device to the IoT Hub service
- Receive messages sent to a device through the IoT Hub service
- Upload a file from the device to a blob

The service samples show how an application or service can manage and communicate with devices through the IoT Hub service:
- Get/Create/Update/Remove devices from the IoT Hub service
- Import/Export devices in bulk to/from the IoT Hub service
- Send a message to a specific device
- Be notified of file uploads by devices

All samples will exit with a return code of 0 if they ran successfully or 1 if they ran into an error.

<a name="Prerequisites"></a>
# Prerequisites

You should have the following items ready before beginning the process:
-   Computer with Git client installed and access to the
    [azure-iot-sdks](https://github.com/Azure/azure-iot-sdks) GitHub public repository.
-   [Prepare your development environment](node-devbox-setup.md).
-   [Setup your IoT hub][lnk-setup-iot-hub]
-   [Provision a test device and get its credentials][lnk-manage-iot-hub]

<a name="DeviceSamples"></a>
# Getting and running the device samples

- The samples files are located in the Node.js Device SDK repository: https://github.com/Azure/azure-iot-sdks/tree/master/node/device/samples
- To run the samples you'll need to download:
    - **package.json**
    - any sample file that you'd like to run:
        - **send_message.js** demonstrates sending a message from the device to the service.
        - **send_batch_http.js** demonstrates batching messages together and sending them over HTTP (message-batching is only supported in HTTP for now)
        - **receive_message.js** demonstrates receiving a message from the service on the device.
        - **file_upload.js** demonstrates uploading a file to a blob from the device.
        - **remote_monitoring.js** demonstrates the use of a Node client in the [Remote Monitoring Preconfigured Solution](https://azure.microsoft.com/en-us/documentation/articles/iot-suite-remote-monitoring-sample-walkthrough/)

- Place the files in the folder of your choice on the target machine/device
- Open a new shell or Node.js command prompt and navigate to this folder where you placed the sample files. 
- Run `npm install` to install dependencies that are necessary to run the sample

```
npm install
```

<a name="SendD2C"></a>
## Sending Device-to-Cloud (D2C) messages
**send_message.js** will send messages to your IoT hub, and the **iothub-explorer** utility will display the messages as your IoT hub receives them.

- From the shell or command prompt you used earlier to run the **iothub-explorer** utility, use the following command to receive device-to-cloud messages from the sample application (replace <device-id> with the ID you assigned your device earlier):

```
node iothub-explorer.js <iothub-connection-string> monitor-events <device-id>
```

- Then in another command prompt run the `send_message.js` sample (replace the `<device connection string` section with the device-specific connection string that you obtained when you setup the device):

```
node send_message.js --connectionString <device connection string>
```

*note: on Linux you'll want to enclose the connection string in double quotes because it contains `;` characters*

<a name="ReceiveC2D"></a>
## Receiving Cloud-to-Device (C2D) messages
**receive_message.js** will receive messages from your IoT hub, and the **iothub-explorer** utility will send the messages to your device through IoT Hub.

- From the shell or command prompt you used earlier to run the **iothub-explorer** utility, use the following command to receive device-to-cloud messages from the sample application (replace <device-id> with the ID you assigned your device earlier):

```
node iothub-explorer.js <iothub-connection-string> send <device-id> <message>
```
- Then in another command prompt run the `receive_message.js` sample (replace the `<device connection string` section with the device-specific connection string that you obtained when you setup the device):

```
node receive_message.js --connectionString <device connection string> --forever
```

*note: on Linux you'll want to enclose the connection string in double quotes because it contains `;` characters*

<a name="UploadFile"></a>
## Uploading a file to a blob
**file_upload.js** will upload a file from the device to a blob. 
- From a command prompt run the `file_upload.js` sample with the following parameters:
    - replace the `<device connection string` section with the device-specific connection string that you obtained when you provisioned the device:
    - replace the `<path-to-file>` section with an absolute or relative path to the file you want to upload
    - replace the `<name-of-blob>` section with the name you want to use for the blob. the actual name will be `<deviceId>/blobname`.

```
node file_upload.js --connectionString <device connection string> --filePath <path-to-file> --blobName <name-of-blob>
```

*note: on Linux you'll want to enclose the connection string in double quotes because it contains `;` characters*

*note: Your must associate a Storage account with your IoT Hub instance in the Azure Portal first*

## Experimenting with various transport protocols and other samples options
- The same samples can be used to test AMQP, AMQP over Websockets, HTTP and MQTT, by passing respectively `--amqp`, `--amqpws`, `--http` or `--mqtt` options to the samples.
- It is possible to use a shared access signature instead of a connection string by using the `--sas <Signature>` option instead of `--connectionString`.
- Each sample may have specific options that can be discovered by calling the sample with no options at all.

# Getting and running the service samples

- The samples files are located in the Node.js Service SDK repository: https://github.com/Azure/azure-iot-sdks/tree/master/node/service/samples
- To run the samples you'll need to download:
    - **package.json**
    - any sample file that you'd like to run:
        - **send_c2d_message.js** demonstrates sending a message to a device from an application or service through the IoT Hub service.
        - **receive_file_notifications.js** demonstrates uploading a file to a blob from the device.
        - **registry_sample.js** demonstrates the use of the service APIs to create, get, and delete devices from your IoT Hub instance device registry.
        - **registry_bulk_sample.js** demonstrate the importing or exporting devices in bulk to/from your IoT Hub instance device registry.

- Place the files in the folder of your choice on the target machine/device
- Open a new shell or Node.js command prompt and navigate to this folder where you placed the sample files. 
- Run `npm install` to install dependencies that are necessary to run the sample

```
npm install
```

## Sending a message to a device (cloud-to-device/C2D)
**send_c2d_message.js** will send messages to a specific device through your IoT hub.

- In a command prompt run the `send_message.js` sample (replace the `<iothub connection string` section with the IoT Hub connection string that you obtained when you provisioned your hub):

```
node send_c2d_message.js --connectionString <iothub connection string> --to <deviceId> --msg <message>
```

- You can receive the message that was just sent using the `receive_message.js` sample of the Device SDK (see: [Receiving Cloud-to-Device (C2D) messages](ReceiveC2D))

*note: on Linux you'll want to enclose the connection string in double quotes because it contains `;` characters*

## Listening for file upload notifications
**receive_file_notification.js** will listen to the IoT Hub service endpoint that signals the results of file uploads for all devices associated with the hub.

- In a command prompt, run the `receive_file_notifications.js` sample (replace the `<iothub connection string` section with the IoT Hub connection string that you obtained when you provisioned your hub)

```
node receive_file_notifications.js --connectionString <iothub connection string>
```

*note: on Linux you'll want to enclose the connection string in double quotes because it contains `;` characters*

*note: Your must associate a Storage account with your IoT Hub instance in the Azure Portal first*

## Running CRUD operations on the device registry
**registry_sample.js** will list the devices registered on your IoT Hub instance, then create a new one, get its information, and finally delete it.

```
node registry_sample.js --connectionString <iothub connection string>
```

*note: on Linux you'll want to enclose the connection string in double quotes because it contains `;` characters*

## Import or export devices in bulk
**registry_bulk_sample.js** will import or export devices to/from the registry depending of the arguments passed to the script. This feature uses Azure Storage Blobs to hold files containing a list of devices either to import 
or that have been exported by a job that runs on the server. The goal of the sample is to demontrate how to setup the import or export job and upload or download that file from the blob.

```
node registry_bulk_sample.js --import --connectionString <iothub connection string> --storageConnectionString <storage connection string>
node registry_bulk_sample.js --export --outFile devices.txt --connectionString <iothub connection string> --storageConnectionString <storage connection string>
```
*note: on Linux you'll want to enclose the connection strings in double quotes because it contains `;` characters*

# Debugging the samples (and/or your code)
[Visual Studio Code](https://code.visualstudio.com/) provides an excellent environment to write and debug Node.js code:
- [Debugging with Visual Studio Code](../../doc/get_started/node-debug-vscode.md)

[lnk-setup-iot-hub]: ../setup_iothub.md
[lnk-manage-iot-hub]: ../manage_iot_hub.md
[lnk-setup-devbox]: node-devbox-setup.md
