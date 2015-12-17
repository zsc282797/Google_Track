var obj = {
	name: 'Mike Song',
	greet: function(){
		console.log(`Hello ${this.name}`);
	}
}

obj.greet();
obj.greet.call({name: 'Zhancheng'}); // use call so that we can override this pointer
     	 	 	 	 	 	 	 	 // pass variables in a "," seperate list
obj.greet.apply({name:'Zhancheng'}); // pass variable in array