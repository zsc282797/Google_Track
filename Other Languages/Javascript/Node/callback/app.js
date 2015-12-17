function greet (callback) {
	
	console.log("Hello");
	callback();
}

greet(function(){
	console.log("callback was invoked!");
	
});