var http = require('http');
var Router = require('node-simple-router');

var common = require("./common");


// Загрузим и закешируем определения модулей
common.MetaOfModules.get(function(res){
    console.log('Module definitions cached');
});



var routes = require("./useFiles");
//var routes = require("./useMongo");


var router = new Router();

router.get('/metamodules', common.metamodules);
router.get('/getconfigs', routes.getconfigs);
router.get('/gethoststatus', common.gethoststatus);
router.get('/saveconfig', routes.saveconfig);
router.get('/delconfig', routes.delconfig);
router.post('/runhosts', routes.runhosts);
router.post('/stophosts', routes.stophosts);


//router.get('/droneconfig', routes.droneconfig);
//router.get('/getconfig/:id', routes.getconfig);

var server = http.createServer(router);
server.listen(process.env.PORT || 4000);
