
/**
 * Module dependencies.
 */
var fs = require('fs');
var express = require('express');
var routes = require('./routes');
var http = require('http');
var path = require('path');
var WebSocketServer = require('ws').Server;
var spawn = require('child_process').spawn;

var mongo = require('mongodb');
var monk = require('monk');
var db = monk('localhost:27017/test');

var app = express();

// all environments
app.set('port', process.env.PORT || 3000);
app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'jade');
app.use(express.favicon(path.join(__dirname, 'public/favicon.ico')));
app.use(express.logger('dev'));
app.use(express.json());
app.use(express.urlencoded());
app.use(express.methodOverride());
app.use(express.cookieParser('your secret here'));
app.use(express.session());
app.use(app.router);
app.use(express.static(path.join(__dirname, 'public')));

// development only
if ('development' == app.get('env')) {
  app.use(express.errorHandler());
}

app.get('/', function (req, res) { res.redirect(302, '/index.html'); });
app.get('/', function(req,res){res.redirect("/index.html")});
app.get('/droneconfig', routes.droneconfig);
app.get('/metamodules', routes.metamodules(db));
app.put('/saveconfig/:id', routes.saveconfig(db));
app.get('/getconfigs', routes.getconfigs(db));
app.post('/delconfig', routes.delconfig(db));
app.post('/runhosts', routes.runhosts(db));
app.post('/stophosts', routes.stophosts(db));
app.get('/gethoststatus', routes.gethoststatus);


// Загружает определения модулей из файлов *.def.js из каталогов модулей и запихивает информацию из них в БД mongo
function loadModuleDefs (dir, done) {
    var results = [];
    fs.readdir(dir, function(err, list) {
        if (err) return done(err);
        var pending = list.length;
        if (!pending) return done(null, results);
        list.forEach(function(file) {
            var fullFile = dir + '/' + file;
            fs.stat(fullFile, function(err, stat) {
                if (stat && stat.isDirectory()) {
                    var configFile = fullFile + '/' + file + '.def.json';
                    fs.readFile(configFile, 'utf8', function (err,data) {
                        if (err) {
                            console.log("Not found definition file " + configFile + " for module '" + file + "'");
                            return;
                        }
                        try {
                            var objModuleDef = JSON.parse(data);

                            var modulesDefs = db.get('modules_defs');
                            modulesDefs.update({"name": objModuleDef.name, "version": objModuleDef.version}, objModuleDef, {"upsert":true }, function (err,count){
                                if(err)
                                {
                                    return console.log(err);
                                }
                                console.log("Load module " + objModuleDef.name + " - OK.");
                            });
                        } catch(e) {
                            console.log("Error parse file " + configFile + " (" + e + ")");
                        }
                    });
                }
            });
        });
    });
};

loadModuleDefs("../../modules", function(err, results) {
    if (err) throw err;
    console.log(results);
});

var server = http.createServer(app);

server.listen(app.get('port'), function(){
  console.log('Express server listening on port ' + app.get('port'));
});

var timerProcStat = undefined;
var wss = new WebSocketServer({server: server});
global.ws_server=undefined;
wss.on('connection', function(ws) {

    global.ws_server = ws;

    ws.on('close', function() {
        global.ws_server = undefined;
        console.log('client websocket disconnect');
    });

    if(timerProcStat===undefined){
//        timerProcStat = setInterval(function() {
//
//
//            fs.readFile('/proc/xenomai/stat', 'utf8', function (err, data) {
//                if (err) {
//                    console.log('Not read file /proc/xenomai/stat. Error:' + err);
//                    return;
//                }
//                if(global.ws_server==undefined) return;
//
//                //console.log(data.split('\n')[1].match(/([1-9]*)\.([1-9])/g)[0]);
//
//                global.ws_server.send(
//                    JSON.stringify({
//                        process: 'OS',
//                        type: 'stat',
//                        data:{
//                            proc: (100-data.split('\n')[1].match(/(\d)+(\.)?(\d)?(?=(\ )+ROOT\/0)/g)[0]).toFixed(1)
//                        }
//                    }), function() {  });
//            });
//
//
//        }, 1000);
    }
});


var telemetry_service;
var i2c_service;
function StartTelemetryService() {
        if(telemetry_service!=undefined){
            return;
        }

    var servicePath = '/usr/local/linuxdrone/services/telemetry';
    telemetry_service = spawn(servicePath);

    telemetry_service.on('error', function (data) {
        if(data.code==="ENOENT"){
            console.log('Not run service: ' + servicePath);
            if(global.ws_server==undefined) return;
            global.ws_server.send(
                JSON.stringify({
                    process: 'nodejs',
                    type: 'error',
                    data:'error run telemetry'
                }), function() {  });
        }
    });


        telemetry_service.stdout.on('data', function (data) {
            if(global.ws_server==undefined) return;
            global.ws_server.send(
                JSON.stringify({
                    process: 'telemetry',
                    type: 'stdout',
                    data:data
                }), function() {  });
            console.log('stdout: ' + data);
        });

        telemetry_service.stderr.on('data', function (data) {
            if(global.ws_server==undefined) return;
            global.ws_server.send(
                JSON.stringify({
                    process: 'telemetry',
                    type: 'stderr',
                    data:data
                }), function() { /* ignore errors */ });
            console.log('stderr: ' + data);
        });

        telemetry_service.on('close', function (code) {
            telemetry_service=undefined;
            if(global.ws_server==undefined) return;
            global.ws_server.send(
                JSON.stringify({
                    process: 'telemetry',
                    type: 'status',
                    data: 'stopped'
                }), function() {  });
            console.log('telemetry child process exited with code ' + code);
        });

        if(telemetry_service!=undefined){
            if(global.ws_server==undefined) return;
            global.ws_server.send(
                JSON.stringify({
                    process: 'telemetry',
                    type: 'status',
                    data: 'running'
                }), function() {  });
        }
};

function Starti2cService() {
    if(i2c_service!=undefined){
        return;
    }

    var servicePath = '/usr/local/linuxdrone/services/i2c_service';
    i2c_service = spawn(servicePath);

    telemetry_service.on('error', function (data) {
        if(data.code==="ENOENT"){
            console.log('Not run service' + servicePath);
            if(global.ws_server==undefined) return;
            global.ws_server.send(
                JSON.stringify({
                    process: 'nodejs',
                    type: 'error',
                    data:'error run i2c_service'
                }), function() {  });
        }
    });


    i2c_service.stdout.on('data', function (data) {
        if(global.ws_server==undefined) return;
        global.ws_server.send(
            JSON.stringify({
                process: 'i2c_service',
                type: 'stdout',
                data:data
            }), function() {  });
        console.log('stdout: ' + data);
    });

    i2c_service.stderr.on('data', function (data) {
        if(global.ws_server==undefined) return;
        global.ws_server.send(
            JSON.stringify({
                process: 'i2c_service',
                type: 'stderr',
                data:data
            }), function() { /* ignore errors */ });
        console.log('stderr: ' + data);
    });

    i2c_service.on('close', function (code) {
        if(global.ws_server==undefined) return;
        i2c_service=undefined;
        global.ws_server.send(
            JSON.stringify({
                process: 'i2c_service',
                type: 'status',
                data: 'stopped'
            }), function() {  });
        console.log('i2c_service child process exited with code ' + code);
    });

    if(i2c_service!=undefined){
        if(global.ws_server==undefined) return;
        global.ws_server.send(
            JSON.stringify({
                process: 'i2c_service',
                type: 'status',
                data: 'running'
            }), function() {  });
    }
};


//StartTelemetryService();
//Starti2cService();
