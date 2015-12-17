var EventEmitter = require('events');
var util = require('util');

function Greetr(){
	EventEmitter.call(this); //make a call to the super's constructor 
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