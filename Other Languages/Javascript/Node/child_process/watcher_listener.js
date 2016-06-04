"use strict";
const
	fs = require('fs'),
	spawn = require('child_process').spawn, 
	filename = process.argv[2];
if (!filename) {
throw Error("A file to watch must be specified!");
}   //Safety cap as we need a file name to finish the process

fs.watch(filename,function(){
	let 
		ls = spawn('ls' , ['-lh', filename]),   //spawn('command', '[list_of_arguments]')
		output = "";
	ls.stdout.on('data',function(chunk){  // on as "upon" in english ,when a certain event is detected 
										  // use 'on' as a keyword to open a listender for an event

		output += chunk.toString();
	});


	ls.on('close', function(){    //the close event is emiited by the ls process of child_process class which extends event emiiter 
		
		let parts = output.split(/\s+/); 
		console.dir([parts[0], parts[4], parts[8]]);
	});

});



console.log("Now watching " + filename + " for changes...");