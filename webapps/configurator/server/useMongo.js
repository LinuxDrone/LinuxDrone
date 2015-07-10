var _ = require('underscore');
var spawn = require('child_process').spawn;
var commonModuleParams = require('../client/ModulesCommonParams.def.js');

//var mongo = require('mongodb');
var monk = require('monk');
//var db = monk('localhost:27017/test');
var db = monk('vrubel.linuxdrone.org:27017/test');


exports.droneconfig = function (req, res) {
    res.render('droneconfig', { title: 'Linuxdrone Configurator' });
};

exports.newconfig = function (req, res) {

    var collection = db.get('visual_configuration');

    var obj = JSON.parse(req.body.records);

    delete obj._id;

    var wr = collection.insert(obj, function (err, docs) {
        if (err) {
            if (req.body.callback) {
                res.writeHead(400, {"Content-Type": "application/javascript"});
                res.write(req.body.callback + '(');
            } else {
                res.writeHead(400, {"Content-Type": "application/json"});
            }
            res.write(JSON.stringify(err));
        } else {
            if (req.body.callback) {
                res.writeHead(200, {"Content-Type": "application/javascript"});
                res.write(req.body.callback + '(');
            } else {
                res.writeHead(200, {"Content-Type": "application/json"});
            }
            res.write(JSON.stringify(docs));
        }

        if (req.body.callback) {
            res.write(')');
        }
        res.end();
    });



    /*
     collection.findOne({_id:id}, {}, function(o, schema){
     db.get('modules_defs').find({}, {}, function (e, metaModules) {
     if(!schema.modulesParams){
     var logMsg = 'In request, not found required property "modulesParams"';
     console.log(logMsg);
     res.send({"success": false, "message":logMsg});
     return;
     }
     var configuration = ConvertGraph2Configuration(JSON.parse(schema.jsonGraph), schema.modulesParams, metaModules);
     configuration.version = schema.version;
     configuration.name = schema.name;

     var configurations = db.get('configuration');
     configurations.update({"name": schema.name, "version": schema.version}, configuration, {"upsert": true }, function (err, count) {
     if (err) {
     res.send({"success": false, "message":"There was a problem adding the information to the database."});
     return console.log(err);
     }
     console.log("Save LinuxDrone configuration " + schema.name + "\\" + schema.version + " - OK.");
     });
     res.send({"success": true});
     });
     });
     */
};

exports.saveconfig = function (req, res) {
    var collection = db.get('visual_configuration');

    var record = JSON.parse(req.body.records);

    var id = record._id;
    delete record._id;

    collection.update({"_id": id}, { $set: record }, {"upsert": true }, function (err, count) {
        if (err) {
            res.send({"success": false, "message": "There was a problem adding the information to the database."});
            return console.log(err);
        }
        console.log("Save visual configuration for " + id + " - OK.");

        collection.findOne({_id: id}, {}, function (o, schema) {
            db.get('modules_defs').find({}, {}, function (e, metaModules) {
                if (!schema.modulesParams) {
                    var logMsg = 'In request, not found required property "modulesParams"';
                    console.log(logMsg);
                    res.send({"success": false, "message": logMsg});
                    return;
                }
                var configuration = ConvertGraph2Configuration(JSON.parse(schema.jsonGraph), schema.modulesParams, metaModules);
                configuration.version = schema.version;
                configuration.name = schema.name;

                var configurations = db.get('configuration');
                configurations.update({"name": schema.name, "version": schema.version}, configuration, {"upsert": true }, function (err, count) {
                    if (err) {
                        res.send({"success": false, "message": "There was a problem adding the information to the database."});
                        return console.log(err);
                    }
                    console.log("Save LinuxDrone configuration " + schema.name + "\\" + schema.version + " - OK.");
                });

                if (req.body.callback) {
                    res.writeHead(200, {"Content-Type": "application/javascript"});
                    res.write(req.body.callback + '({"success": true})');
                } else {
                    res.writeHead(200, {"Content-Type": "application/json"});
                    res.write('{"success": true}');
                }
                res.end();
            });
        });
    });
}

exports.delconfig = function (db) {
    return function (req, res) {
        var collection = db.get('visual_configuration');
        collection.findOne({_id: req.body._id}, {}, function (o, schema) {
            var configurations = db.get('configuration');
            configurations.remove({"name": req.body.name, "version": req.body.version});
        });

        collection.remove({"_id": req.body._id});

        res.send({"success": true});
    };
};

exports.getconfigs = function (req, res) {
    var collection = db.get('visual_configuration');
    collection.find({}, {}, function (e, docs) {
        if (req.body.callback) {
            res.writeHead(200, {"Content-Type": "application/javascript"});
            res.write(req.body.callback + '(' + JSON.stringify(docs) + ')');
        } else {
            res.writeHead(200, {"Content-Type": "application/json"});
            res.write(JSON.stringify(docs));
        }
        res.end();
    });
};

exports.getconfig = function (db) {
    return function (req, res) {
        var collection = db.get('visual_configuration');
        collection.findOne({_id: req.params.id}, {}, function (o, schema) {
            res.setHeader("Content-Type", "application/json");
            res.setHeader("Content-Disposition", "attachment;filename='" + schema.name + "-" + schema.version + ".json'");
            res.json(schema);
        });
    };
};


var chost = undefined;


var hostStatus = {
    status: 'stopped', // running
    schemaName: '',
    schemaVersion: ''
};


exports.gethoststatus = function (req, res) {
    //return;
    res.json(hostStatus);
};


exports.runhosts = function (db) {
    return function (req, res) {

        if (hostStatus.status === 'running') {
            res.send({"success": false, message: "Host already running"});
            return;
        }

        try {
            chost = spawn('/usr/local/linuxdrone/bin/c-host', [req.body.name, req.body.version]);
        } catch (e) {
            console.log(e);
            res.send({"success": false, "message": e});
            return;
        }


        hostStatus.status = 'running';
        hostStatus.schemaName = req.body.name;
        hostStatus.schemaVersion = req.body.version;


        chost.stdout.on('data', function (data) {
            if (global.ws_server == undefined) return;
            global.ws_server.send(
                JSON.stringify({
                    process: 'c-host',
                    type: 'stdout',
                    data: data
                }), function () {
                });
            console.log('stdout: ' + data);
        });

        chost.stderr.on('data', function (data) {
            if (global.ws_server == undefined) return;
            global.ws_server.send(
                JSON.stringify({
                    process: 'c-host',
                    type: 'stderr',
                    data: data
                }), function () { /* ignore errors */
                });
            console.log('stderr: ' + data);
        });

        chost.on('close', function (code) {
            chost = undefined;

            hostStatus.status = 'stopped';
            if (global.ws_server != undefined) {
                global.ws_server.send(JSON.stringify(hostStatus), function () {
                });
            }
            hostStatus.schemaName = '';
            hostStatus.schemaVersion = '';

            console.log('c-host child process exited with code ' + code);
        });

        chost.on('error', function (code) {
            chost = undefined;

            hostStatus.status = 'stopped';
            if (global.ws_server != undefined) {
                global.ws_server.send(JSON.stringify(hostStatus), function () {
                });
            }
            hostStatus.schemaName = '';
            hostStatus.schemaVersion = '';

            console.log('c-host child process exited with code ' + code);
        });


        if (global.ws_server != undefined) {
            global.ws_server.send(JSON.stringify(hostStatus), function () {
            });
        }

        res.send({"success": true});
    };
};

exports.stophosts = function (db) {
    return function (req, res) {

        if (chost == undefined) {
            res.send({"success": false, message: "Host already stopped"});
            return;
        }

        try {
            chost.kill();
        } catch (e) {
            console.log(e);
            res.send({"success": false, "message": e});
            return;
        }
        chost = undefined;

        /*
         hostStatus.status = 'stopped';
         if (global.ws_server != undefined) {
         global.ws_server.send(JSON.stringify(hostStatus), function () {
         });
         }
         hostStatus.schemaName = '';
         hostStatus.schemaVersion = '';
         */
        res.send({"success": true});
    };
};

// Конвертирует визуальное представление графа в конфигурацию модулей принятую в linuxdrone
function ConvertGraph2Configuration(graph, modulesParams, metaModules) {
    var config = {
        "type": "configuration",
        "modules": new Array(),
        "links": new Array()
    };

    graph.cells.forEach(function (cell) {
        ConvertVisualCell(graph, config.modules, config, cell, modulesParams, metaModules);
    });

    //console.log(config);
    return config;
}


function CastValue2Type(value, type) {
    switch (type) {
        case "char":
        case "short":
        case "int":
        case "long":
        case "long long":
        case "float":
        case "double":
            value = Number(value);
            break;

        case "const char*":
            break;

        case "bool":
            if (_.isString(value) && value === "false") {
                value = false;
            }
            else {
                value = Boolean(value);
            }
            break;

        default:
            console.log("CastValue2Type Unknown type: " + type);
            break;
    }

    return value;
}


function ConvertVisualCell(graph, arModules, arLinks, cell, modulesParams, metaModules) {
    if (cell.type == "devs.Model") {

        var module = {
            "name": cell.moduleType,
            /* Название instance модуля*/
            "instance": cell.attrs[".label"].text
        };

        var metaModule = _.find(metaModules, function (meta) {
            return meta.name == cell.moduleType;
        });

        // Перенос общих (определенных для всех типов модулей) параметров
        if (modulesParams && modulesParams[module.instance].common) { //&& modulesParams[module.instance]
            var commonParams = modulesParams[module.instance].common;
            Object.keys(commonParams).forEach(function (paramName) {
                var metaParam = _.find(commonModuleParams.commonModuleParamsDefinition, function (meta) {
                    return meta.name == paramName;
                });
                if (!metaParam) {
                    console.log("In instance " + module.instance + ". Not found metadata for parameter '" + paramName + "'  in common definition for modules ");
                }
                var typedValue = CastValue2Type(commonParams[paramName], metaParam.type);
                if (typedValue === undefined) {
                    console.log("Unknown type '" + metaParam.type + "' in metadata for parameter '" + paramName + "' in common definition for modules");
                }
                module[paramName] = typedValue;
            });
        }

        // Перенос специфических для модуля параметров
        var specificParams = modulesParams[module.instance].specific;
        if (specificParams) {
            module.params = {};
            Object.keys(specificParams).forEach(function (paramName) {
                var metaParam = _.find(metaModule.paramsDefinitions, function (meta) {
                    return meta.name == paramName;
                });
                if (!metaParam) {
                    console.log("Not found metadata for parameter '" + paramName + "' for module '" + cell.moduleType + "'");
                }
                else {
                    var typedValue = CastValue2Type(specificParams[paramName], metaParam.type);
                    if (typedValue === undefined) {
                        console.log("Unknown type '" + metaParam.type + "' in metadata for parameter '" + paramName + "' for module " + cell.moduleType + "'");
                    }
                    module.params[paramName] = typedValue;
                }
            });
        }

        // Перенос конфигурации сложных модулей
        if (cell.blocksJSON) {
            module.blocksConfig = {"blocks": new Array(), "links": new Array()};

            cell.blocksJSON.cells.forEach(function (blockInstance) {
                var blockParams = modulesParams[module.instance].blocksConfig;
                var metaBlocks = metaModule.subSchema.blocksDefinitions;

                ConvertVisualCell(cell.blocksJSON, module.blocksConfig.blocks, module.blocksConfig, blockInstance, blockParams, metaBlocks);
            });
        }

        arModules.push(module);
    }

    if (cell.type == "link") {
        arLinks.links.push({
            "type": cell.mode,
            "portType": cell.portType, // Тип значения порта
            "outInst": GetInstanceName(graph, cell.source.id),
            "inInst": GetInstanceName(graph, cell.target.id),
            "outPin": cell.source.port,
            "inPin": cell.target.port,
            "nameOutGroup": cell.nameOutGroup
        });
    }
}


// Возвращает название инстанса по идентификатору
function GetInstanceName(graph, id) {
    var res;
    graph.cells.forEach(function (cell) {
        if (cell.id == id) {
            res = cell.attrs[".label"].text;
        }
    });
    return res;
}
