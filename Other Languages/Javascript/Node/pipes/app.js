var fs = require('fs');
var zlib = require('zlib');

var readable = fs.createReadStream(__dirname + '/greet.txt');
var writable = fs.createWriteStream(__dirname + '/greetcpy.txt');

var compressed = fs.createWriteStream(__dirname + '/greet.txt.gz');

var gzip = zlib.createGzip();


readable.pipe(writable);
readable.pipe(gzip).pipe(compressed); // method chaining to the next pipe
