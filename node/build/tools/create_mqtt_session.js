// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

'use strict';

var Protocol = require('azure-iot-device-mqtt').Mqtt;
var Client = require('azure-iot-device').Client;
var argv = require('yargs')
             .usage('Usage: \r\nnode $0 --connectionString <DEVICE CONNECTION STRING>\r\nnode $0 --sas <SHARED ACCESS SIGNATURE>')
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
             .argv;

// Create the client instance, either with a connection string or a shared access signature
var client = argv.connectionString ? Client.fromConnectionString(argv.connectionString, Protocol)
                                   : Client.fromSharedAccessSignature(argv.sas, Protocol);

// Open the connection to the server
client.open(printResultFor('client.open', function(err, result) {
  // Subscribe to the message event that will fire every time the device receives a message
  client.on('message', function () {});
  process.exit(0);
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