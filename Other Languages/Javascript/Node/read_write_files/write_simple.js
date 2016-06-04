const fs = require('fs'); 
var message = "This is written by node.js!";
fs.writeFile('target.txt',message,function(err){
	if (err){
		throw err;
	}	
	console.log("Message wrote to the File");
});