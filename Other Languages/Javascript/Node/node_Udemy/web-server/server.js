var express = require('express');
var app = express();
var middleware = require('./middleware.js');


app.use(middleware.logger);
app.get('/',function (req,res){

  res.send("Hello Express");

});

app.get('/about',function(req,res) {

  res.send("About us!");

});

app.use(express.static(__dirname + '/public'));
app.listen(3000, function(){

  console.log("server running on port 3000");
});
