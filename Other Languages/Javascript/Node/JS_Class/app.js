'use strict'; // required for using classes in Node (ES6)
class Person {
	constructor(firstname, lastname){
		this.firstname = firstname;
		this.lastname = lastname;
		
	}
	greet(){
		console.log("Hello from "+this.firstname+ ' '+this.lastname);
		}
	
}

var mike = new Person('Mike','Song');
mike.greet();