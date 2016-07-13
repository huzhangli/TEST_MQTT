// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

'use strict';

var Amqp = require('azure-iot-device-amqp').Amqp;
var Client = require('azure-iot-device').Client;
var fs = require('fs');
var argv = require('yargs')
             .usage('Usage:\r\nnode $0 --connectionString <DEVICE CONNECTION STRING> --filePath <FILE TO UPLOAD> --blobName <BLOB NAME>\r\nnode $0 --sas <SHARED ACCESS SIGNATURE> --filePath <FILE TO UPLOAD> --blobName <BLOB NAME>')
             .demand(['filePath','blobName'])
             .check(function(argv, opts) { 
               if(!argv.connectionString && !argv.sas || argv.connectionString && argv.sas) { 
                 throw new Error('Please specify either a connection string or a shared access signature.');
               } else {
                 return true;
               }
             })
             .alias('f', 'filePath')
             .describe('filePath', 'Path to the file that shall be uploaded.')
             .alias('s', 'sas')
             .describe('sas', 'Device-specific shared access signature.')
             .alias('b', 'blobName')
             .describe('blobName', 'Name of the blob to upload the file to.')
             .argv;

// fromConnectionString/fromSharedAccessSignature must specify a transport constructor, coming from any transport package.
var client = argv.connectionString ? Client.fromConnectionString(argv.connectionString, Amqp)
                                   : Client.fromSharedAccessSignature(argv.sas, Amqp);

fs.stat(argv.filePath, printResultFor('file.stat', function (fileStats) {
  var fileStream = fs.createReadStream(argv.filePath);
  console.log('uploading ' + argv.filePath);
  client.uploadToBlob(argv.blobName, fileStream, fileStats.size, printResultFor('client.uploadToBlob', function () {
    fileStream.destroy();
    process.exit(0);
  }));
}));

function printResultFor(operation, next) {
  return function(err, result) {
    if(err) {
      console.error(operation + ' failed: ' + err.constructor.name + ': ' + err.message);
      process.exit(1);
    } else {
      console.log(operation + ' succeeded');
      if (next) next(result);
    }
  };
};