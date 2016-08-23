
var weather = require('./weather.js');
var location = requre('./location.js');
weather(function(currentWeather){
      console.log(currentWeather);


});

/*request(  {url: url,
          json:true },
          function (error,message,body){
              if (error){
                console.log("Unable to accees data");
                }
              else {
                console.log(JSON.stringify(body,null,4));
                console.log('It\'s' + body.main.temp );
              }
          }

      );
*/
