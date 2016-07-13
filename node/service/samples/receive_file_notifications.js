// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

'use strict';

var Client = require('azure-iothub').Client;
var argv = require('yargs')
             .usage('Usage: node $0 --connectionString <IOTHUB CONNECTION STRING>')
             .demand(['connectionString'])
             .alias('c', 'connectionString')
             .describe('connectionString', 'IoT Hub service connection string.')
             .argv

var client = Client.fromConnectionString(argv.connectionString);

client.open(printResultFor('client.open', function() {
  client.getFileNotificationReceiver(printResultFor('client.getFileNotificationReceiver', function(receiver) {
    receiver.on('message', function(msg) {
      console.log('File uploaded: ');
      console.log(msg.data.toString());
      receiver.complete(msg, printResultFor('receiver.complete', function() { process.exit(0); }));
    });
  }));
}));

function printResultFor(operation, next) {
  return function(err, result) {
    if(err) {
      console.error(operation + ' failed: ' + err.constructor.name + ': ' + err.message);
      process.exit(1);
    } else {
      console.log(operation + ' succeeded: ' + result.constructor.name);
      if (next) next(result);
    }
  };
};