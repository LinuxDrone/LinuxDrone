var http = require('http');
var Router = require('node-simple-router');

var methods = require("./useFiles");

var router = new Router();


router.get("/metamodules", methods.metamodules);
router.get("/hello2", methods.hello2);



var server = http.createServer(router);
server.listen(process.env.PORT || 4000);