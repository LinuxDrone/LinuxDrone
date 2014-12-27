var http = require('http');
var Router = require('node-simple-router');

var common = require("./common");

//var routes = require("./useFiles");
var routes = require("./useMongo");


var router = new Router();

router.get('/droneconfig', routes.droneconfig);
router.get('/metamodules', common.metamodules);
router.post('/newconfig', routes.newconfig);
router.put('/saveconfig/:id', routes.saveconfig);
router.get('/getconfig/:id', routes.getconfig);
router.get('/getconfigs', routes.getconfigs);
router.delete('/delconfig/:id', routes.delconfig);
router.post('/runhosts', routes.runhosts);
router.post('/stophosts', routes.stophosts);
router.get('/gethoststatus', common.gethoststatus);


var server = http.createServer(router);
server.listen(process.env.PORT || 4000);


