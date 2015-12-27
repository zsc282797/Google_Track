var express = require("express");
var bodyParser = require("body-parser");


var app = express();
var port = process.env.PORT || 3000;

var urlendcodedParser = bodyParser.urlencoded({extended:false});

app.use('/asset', express.static(__dirname + '/public'));
app.set('view engine','ejs');



app.get('/',function(req,res) {
	
	res.render('index');
});

app.get('/api',function(req,res){
	res.json({firsname: 'Mike', lastname: 'Song'});
	
});

app.get('/person/:id',function(req,res) {
	
	res.render('person',{ID: req.params.id, QStr: req.query.qstr });
});

app.post('/person',urlendcodedParser,function(req,res){
	res.send('Thank you !');
	console.log(req.body);
	
});




app.listen(port);