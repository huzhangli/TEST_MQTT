// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

'use strict';

var Protocol = require('azure-iot-device-amqp').Amqp;
var Client = require('azure-iot-device').Client;
var argv = require('yargs')
             .usage('Usage: \r\nnode $0 --connectionString <DEVICE CONNECTION STRING> [--amqp] [--mqtt] [--http] [--amqpws]\r\nnode $0 --sas <SHARED ACCESS SIGNATURE> [--amqp] [--mqtt] [--http] [--amqpws] [--forever]')
             .check(function(argv, opts) { 
               if(!argv.connectionString && !argv.sas || argv.connectionString && argv.sas) { 
                 throw new Error('Please specify either a connection string or a shared access signature.');
               } else {
                 return true;
               }
             })
             .alias('c', 'connectionString')
             .describe('connectionString', 'Device-specific connection string.')
             .alias('s', 'sas')
             .describe('sas', 'Device-specific shared access signature.')
             .boolean('forever')
             .alias('f', 'forever')
             .argv;

if (argv.mqtt) {
  console.log('using MQTT');
  Protocol = require('azure-iot-device-mqtt').Mqtt;
} else if (argv.http) {
  console.log('using HTTP');
  Protocol = require('azure-iot-device-http').Http;
} else if (argv.amqpws) {
  console.log('using AMQP over Websockets');
  Protocol = require('azure-iot-device-amqp-ws').AmqpWs;
} else {
  console.log('using AMQP');
}

console.log('Waiting for message(s)');

// Create the client instance, either with a connection string or a shared access signature
var client = argv.connectionString ? Client.fromConnectionString(argv.connectionString, Protocol)
                                   : Client.fromSharedAccessSignature(argv.sas, Protocol);

// Open the connection to the server
client.open(printResultFor('client.open', function(err, result) {
  // Subscribe to the message event that will fire every time the device receives a message
  client.on('message', function (msg) {
    console.log('Id: ' + msg.messageId + ' Body: ' + msg.data.toString());
    // Completes the message, effectively removing it from the queue (Reject/Abandon follow the same pattern)
    client.complete(msg, printResultFor('completed', function() {
      if(!argv.forever) {
        process.exit(0);
      }
    }));
  });
}));

function printResultFor(operation, next) {
  return function(err, result) {
    if(err) {
      console.error(operation + ' failed: ' + err.constructor.name + ': ' + err.message);
      process.exit(1);
    } else {
      console.log(operation + ' succeeded: ' + result.constructor.name);
      if (next) next();
    }
  };
};