
/**
 * Module dependencies.
 */
var fs = require('fs');
var express = require('express');
var routes = require('./routes');
var http = require('http');
var path = require('path');

var mongo = require('mongodb');
var monk = require('monk');
var db = monk('localhost:27017/test');

var app = express();

// all environments
app.set('port', process.env.PORT || 3000);
app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'jade');
app.use(express.favicon());
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

app.get('/', routes.index);
app.get('/droneconfig', routes.droneconfig);
app.get('/metamodules', routes.metamodules(db));
app.post('/saveconfig', routes.saveconfig(db));
app.get('/getconfigs', routes.getconfigs(db));
app.post('/delconfig', routes.delconfig(db));

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

http.createServer(app).listen(app.get('port'), function(){
  console.log('Express server listening on port ' + app.get('port'));
});



