var city = 'Shanghai';
var request = require('request');
var url = 'http://api.openweathermap.org/data/2.5/weather?appid=b503ffc0ac8df25a8246fe985db04f00&q=' +
city + '&units=imperial';


module.exports = function(callback){

  callback('Here is some weather!');
  request(  {url: url,
            json:true },
            function (error,message,body){
                if (error){
                  callback("Unable to accees data");
                  }
                else {
                callback(JSON.stringify(body,null,4));
                callback('It\'s' + body.main.temp );
                }
            }

        );



}
