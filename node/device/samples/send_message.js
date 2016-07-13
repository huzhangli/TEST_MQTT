// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

'use strict';

var Protocol = require('azure-iot-device-amqp').Amqp;
var deviceSdk = require('azure-iot-device');
var argv = require('yargs')
             .usage('Usage: \r\nnode $0 --connectionString <DEVICE CONNECTION STRING> [--amqp] [--mqtt] [--http] [--amqpws]\r\nnode $0 --sas <SHARED ACCESS SIGNATURE> [--amqp] [--mqtt] [--http] [--amqpws]')
             .check(function(argv, opts) { 
               if(!argv.connectionString && !argv.sas || argv.connectionString && argv.sas) { 
                 throw new Error('Please specify either a connection string or a shared access signature.');
               } else {
                 return true;
               }
             })
             .alias('c', 'connectionString')
             .alias('s', 'sas')
             .describe('connectionString', 'Device-specific connection string.')
             .describe('sas', 'Device-specific shared access signature.')
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

// fromConnectionString/fromSharedAccessSignature must specify a transport constructor, coming from any transport package.
var client = argv.connectionString ? deviceSdk.Client.fromConnectionString(argv.connectionString, Protocol)
                                   : deviceSdk.Client.fromSharedAccessSignature(argv.sas, Protocol);

client.open(printResultFor('client.open', function() {
  var message = new deviceSdk.Message('Hello, World!');
  client.sendEvent(message, printResultFor('client.send', function() { process.exit(0); }));
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