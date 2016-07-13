// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

'use strict';

var Client = require('azure-iothub').Client;
var Message = require('azure-iot-common').Message;
var argv = require('yargs')
             .usage('Usage: node $0 --connectionString <IOTHUB CONNECTION STRING> --to <DEVICE ID> --msg <MESSAGE>')
             .demand(['connectionString', 'to'])
             .alias('c', 'connectionString')
             .describe('connectionString', 'IoT Hub service connection string.')
             .alias('t', 'to')
             .describe('to', 'ID of the device that shall receive the message')
             .alias('m', 'msg')
             .describe('msg', 'Message that shall be sent')
             .argv;

var messageBody = argv.msg ? argv.msg : 'Hello, World!';
var message = new Message(messageBody);

var client = Client.fromConnectionString(argv.connectionString);

client.open(printResultFor('client.open', function(){
  console.log('Sending message: ' + message.getData());
  client.send(argv.to, message, printResultFor('send', function() { process.exit(0) }));
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