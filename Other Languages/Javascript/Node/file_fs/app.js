var fs = require('fs');
var greet = fs.readFileSync(__dirname + '/greet.txt','utf8');//_dirname is provided when run
//This is Synchrous approach
console.log(greet);

var greet = fs.readFile(__dirname + '/greet.txt',function(err,data){
	
	console.log(data); // the returned value is a buffer instead of string!
	console.log(data.toString());
	//this is a error-first callback function 
	// OR we can provide the 'utf-8 in the parameters'
	
});



