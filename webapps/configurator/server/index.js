var http = require('http');
var Router = require('node-simple-router');

//var routes = require("./useFiles");
var routes = require("./useMongo");

var router = new Router();

router.get('/droneconfig', routes.droneconfig);
router.get('/metamodules', routes.metamodules);
router.post('/newconfig', routes.newconfig);
router.put('/saveconfig/:id', routes.saveconfig);
router.get('/getconfig/:id', routes.getconfig);
router.get('/getconfigs', routes.getconfigs);
router.delete('/delconfig/:id', routes.delconfig);
router.post('/runhosts', routes.runhosts);
router.post('/stophosts', routes.stophosts);
router.get('/gethoststatus', routes.gethoststatus);

//router.get("/metamodules", routes.metamodules);
router.get("/hello2", routes.hello2);



var server = http.createServer(router);
server.listen(process.env.PORT || 4000);