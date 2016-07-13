// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

'use strict';

var iothub = require('azure-iothub');
var azureStorage = require('azure-storage');
var argv = require('yargs')
             .usage('Usage: node $0 [--import] [--export] --connectionString <IOTHUB CONNECTION STRING> --storageConnectionString <STORAGE CONNECTION STRING> [--outFile]')
             .check(function (argv, opts) {
                 if (!!argv.import && !!argv.export || !argv.import && !argv.export) {
                     throw new Error('Either --import or --export must be specified');
                 }

                 if (argv.export && !argv.outFile) {
                   throw new Error('--outFile must be specified for export operations');
                 }

                 return true;
             })
             .demand(['connectionString', 'storageConnectionString'])
             .alias('c', 'connectionString')
             .describe('connectionString', 'IoT Hub service connection string.')
             .alias('s', 'storageConnectionString')
             .describe('storageConnectionString', 'Storage service connection string.')
             .alias('o', 'outFile')
             .argv

var registry = iothub.Registry.fromConnectionString(argv.connectionString);
var blobSvc = azureStorage.createBlobService(argv.storageConnectionString);
var blobName = 'devices.txt';

var startDate = new Date();
var expiryDate = new Date(startDate);
expiryDate.setMinutes(startDate.getMinutes() + 100);
startDate.setMinutes(startDate.getMinutes() - 100);

var inputSharedAccessPolicy = {
  AccessPolicy: {
    Permissions: 'rl',
    Start: startDate,
    Expiry: expiryDate
  },
};

var outputSharedAccessPolicy = {
  AccessPolicy: {
    Permissions: 'rwd',
    Start: startDate,
    Expiry: expiryDate
  },
};

var inputContainerName = 'importcontainer';
var outputContainerName = 'exportcontainer';

if (argv.import) {
  var devices = [
    {
      id: 'aaa1',
      status: 'enabled'
    },
    {
      id: 'aaa2',
      status: 'enabled'
    }
  ];

  ImportDevices(devices, 'createOrUpdate', function(){
    ImportDevices(devices, 'delete', function() {
      process.exit(0);
    });
  });
} else {
  ExportDevices(function() {
    process.exit(0);
  });
}

function ImportDevices(devices, importMode, done) {
  blobSvc.createContainerIfNotExists(inputContainerName, printResultFor('create input container', function() {
    var inputSasToken = blobSvc.generateSharedAccessSignature(inputContainerName, null, inputSharedAccessPolicy);
    var inputSasUrl = blobSvc.getUrl(inputContainerName, null, inputSasToken);
    var deviceString = '';
    devices.forEach(function (device) {
      device.importMode = importMode;
      deviceString += JSON.stringify(device) + '\n';
    });
    blobSvc.createBlockBlobFromText(inputContainerName, blobName, deviceString, printResultFor('create input blob', function() {
      blobSvc.createContainerIfNotExists(outputContainerName, printResultFor('create output container', function() {
        var outputSasToken = blobSvc.generateSharedAccessSignature(outputContainerName, null, outputSharedAccessPolicy);
        var outputSasUrl = blobSvc.getUrl(outputContainerName, null, outputSasToken);
        registry.importDevicesFromBlob(inputSasUrl, outputSasUrl, printResultFor('import devices', function(result) {
          console.log('--------------\r\nDevices Import Job Identifier:--------------\r\n' + result);
          var jobId = JSON.parse(result).jobId;
          var interval = setInterval(function () {
            registry.getJob(jobId, function (error, result) {
              if (error) {
                console.error('Could not get job status: ' + error.message + ' : ' + error.responseBody);
              } else {
                console.log('--------------\r\njob ' + jobId + ' status:\r\n--------------\r\n' + result);
                var status = JSON.parse(result).status;
                if (status === "completed") {
                  clearInterval(interval);
                  done();
                }
              }
            });
          }, 500);
        }));
      }));
    }));
  }));
}

function ExportDevices() {
  blobSvc.createContainerIfNotExists(outputContainerName, printResultFor('create output container', function() {
    var outputSasToken = blobSvc.generateSharedAccessSignature(outputContainerName, null, outputSharedAccessPolicy);
    var outputSasUrl = blobSvc.getUrl(outputContainerName, null, outputSasToken);
    registry.exportDevicesToBlob(outputSasUrl, true, printResultFor('export devices', function(result) {
      console.log('--------------\r\nDevices Export Job Identifier:--------------\r\n' + result);
      var jobId = JSON.parse(result).jobId;
      var interval = setInterval(function () {
        registry.getJob(jobId, function (error, result) {
          if (error) {
            console.error('Could not get job status: ' + error.message + ' : ' + error.responseBody);
          } else {
            console.log('--------------\r\njob ' + jobId + ' status:\r\n--------------\r\n' + result);
            var status = JSON.parse(result).status;
            if (status === "completed") {
              clearInterval(interval);
              blobSvc.getBlobToLocalFile(outputContainerName, blobName, argv.outFile, printResultFor('getBlobToLocalFile', function() { process.exit(0); }));
            }
          }
        });
      }, 500);
    }));
  }));
}

function printResultFor(operation, next) {
    return function (err, result) {
        if (err) {
            console.error(operation + ' failed: ' + err.constructor.name + ': ' + err.message);
            process.exit(1);
        } else {
            console.log(operation + ' succeeded: ' + result.constructor.name);
            if (next) next(result);
        }
    }
}
