var EventEmitter = require('events');
var util = require('util');

function Greetr(){
	this.greeting = "Hello World!";
	
}

util.inherits(Greetr, EventEmitter);

Greetr.prototype.greet = function(){
	console.log(this.greeting);
	this.emit('greet');
		
}

var greeter1 = new Greetr();

greeter1.on('greet',function(){
	
	console.log('someone Greeted!');
});

greeter1.greet();