// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

'use strict';

var iothub = require('azure-iothub');
var argv = require('yargs')
             .usage('Usage: node $0 --connectionString <IOTHUB CONNECTION STRING>')
             .demand(['connectionString'])
             .alias('c', 'connectionString')
             .describe('connectionString', 'IoT Hub service connection string.')
             .argv

var registry = iothub.Registry.fromConnectionString(argv.connectionString);

// List devices
console.log('**listing devices...');
registry.list(function (err, deviceList) {
  deviceList.forEach(function (device) {
    console.log(device.deviceId);
  });

  // Create a new device
  var device = new iothub.Device(null);
  device.deviceId = 'sample-device-' + Date.now();
  console.log('\n**creating device \'' + device.deviceId + '\'');
  registry.create(device, printAndContinue('create', function() {
    // Get the newly-created device
    console.log('\n**getting device \'' + device.deviceId + '\'');
    registry.get(device.deviceId, printAndContinue('get', function() {
      // Delete the new device
      console.log('\n**deleting device \'' + device.deviceId + '\'');
      registry.delete(device.deviceId, printAndContinue('delete', function() {
        process.exit(0);
      }));
    }));
  }));
});

function printAndContinue(op, next) {
  return function (err, deviceInfo, res) {
    if (err) {
      console.log(op + ' error: ' + err.toString());
      process.exit(1);
    }

    if (res) console.log(op + ' status: ' + res.statusCode + ' ' + res.statusMessage);
    if (deviceInfo) console.log(op + ' device info: ' + JSON.stringify(deviceInfo));
    if (next) next();
  };
}