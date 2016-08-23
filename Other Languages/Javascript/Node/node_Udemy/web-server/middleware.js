//request -> middleware -> reqponse/next action

var middleware = {
  requireAuthentication: function(req,res,next){
      console.log("private route hit");
      next();
  },
  logger: function(req, res, next){
    console.log(req.method + ' : ' + req.originalUrl + ' @ ' + new Date().toString());
    next();
  }
};

module.exports = middleware;
//app.use(middleware.requireAuthentication); //application level middleware that
//effect every single route
