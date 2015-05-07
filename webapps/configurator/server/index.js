var http = require('http');
var Router = require('node-simple-router');
var WebSocketServer = require('ws').Server;

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
router.get('/getconfig/:id', routes.getconfig);
router.get('/runhosts', common.runhosts);
router.post('/stophosts', routes.stophosts);


//router.get('/droneconfig', routes.droneconfig);


var server = http.createServer(router);
server.listen(process.env.PORT || 4000);


var timerProcStat = 1;//undefined;
var wss = new WebSocketServer({server: server});
global.ws_server = undefined;
wss.on('connection', function (ws) {

    console.log('new websocket connect\n');

    global.ws_server = ws;

    ws.on('close', function () {
        global.ws_server = undefined;
        console.log('client websocket disconnect');
    });

    if (timerProcStat === undefined) {
        timerProcStat = setInterval(function() {
            fs.readFile('/proc/xenomai/stat', 'utf8', function (err, data) {
                if (err) {
                    console.log('Not read file /proc/xenomai/stat. Error:' + err);
                    return;
                }
                if(global.ws_server==undefined) return;

                //console.log(data.split('\n')[1].match(/([1-9]*)\.([1-9])/g)[0]);

                global.ws_server.send(
                    JSON.stringify({
                        process: 'OS',
                        type: 'stat',
                        data:{
                            proc: (100-data.split('\n')[1].match(/(\d)+(\.)?(\d)?(?=(\ )+ROOT\/0)/g)[0]).toFixed(1)
                        }
                    }), function() {  });
            });
        }, 1000);
    }
});
