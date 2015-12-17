var fs = require('fs');

var readable = fs.createReadStream(__dirname + '/greet.txt',
 {encoding:'utf8',
  highWaterMark:1024 * 16
    });
var writable = fs.createWriteStream(__dirname + '/greetcopy.txt');

readable.on('data',function(chunk){  // file is bigger than buffer, than it is passed by chunck
	console.log(chunk.length);
	writable.write(chunk);
	

	
});
