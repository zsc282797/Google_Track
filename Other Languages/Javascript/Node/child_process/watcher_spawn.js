"use strict";
const
	fs = require('fs'),
	spawn = require('child_process').spawn, 
	filename = process.argv[2];

if (!filename) {
throw Error("A file to watch must be specified!");
}   //Safety cap as we need a file name to finish the process


fs.watch(filename, function() {
	let ls = spawn('ls', ['-lh', filename]); 
	ls.stdout.pipe(process.stdout);  // Need to understand what does the pipe function do ? 
});
console.log("Now watching " + filename + " for changes...");
