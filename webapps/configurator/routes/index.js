var _ = require('underscore');
var spawn = require('child_process').spawn;
var commonModuleParams = require('../public/ModulesCommonParams.def.js');

/*
 * GET home page.
 */
exports.index = function (req, res) {
    //res.render('index', { title: 'Express' });
};

exports.droneconfig = function (req, res) {
    res.render('droneconfig', { title: 'Linuxdrone Configurator' });
};

exports.metamodules = function (db) {
    return function (req, res) {
        db.get('modules_defs').find({}, {}, function (e, metaModules) {
            res.json(metaModules);
        });
    };
};


exports.newconfig = function (db) {
    return function (req, res) {

        var collection = db.get('visual_configuration');

        //delete req.body._id;

        var wr = collection.insert(req.body, function(err,docs) {
                if (err) {
                    res.send(400, err);
                }
                else {
                    res.send(201, docs);
                }
            });


return ;
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
};


exports.saveconfig = function (db) {
    return function (req, res) {
        var collection = db.get('visual_configuration');

        var id = req.body._id;
        delete req.body._id;

        collection.update({"_id": id}, { $set: req.body }, {"upsert": true }, function (err, count) {
            if (err) {
                res.send({"success": false, "message":"There was a problem adding the information to the database."});
                return console.log(err);
            }
            console.log("Save visual configuration for " + id + " - OK.");
        });

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
    };
};

exports.delconfig = function (db) {
    return function (req, res) {
        var collection = db.get('visual_configuration');
        collection.findOne({_id:req.body._id}, {}, function(o, schema){
            var configurations = db.get('configuration');
            configurations.remove({"name": req.body.name, "version": req.body.version});
        });

        collection.remove({"_id": req.body._id});

        res.send({"success": true});
    };
};

exports.getconfigs = function (db) {
    return function (req, res) {
        var collection = db.get('visual_configuration');
        collection.find({}, {}, function (e, docs) {
            res.json(docs);
        });
    };
};


var chost=undefined;
exports.gethoststatus = function (req, res) {
console.log(chost);
    var hoststatus;
    if (chost!=undefined) {
        hoststatus = 'running';
    } else {
        hoststatus = 'stopped';
    }
    res.json(hoststatus);
};


exports.runhosts = function (db) {
    return function (req, res) {

        if(chost!=undefined){
            return;
        }

        res.json(req.body);

        chost = spawn('/usr/local/linuxdrone/bin/c-host', [req.body.name, req.body.version]);

        chost.stdout.on('data', function (data) {
            if(global.ws_server==undefined) return;
            global.ws_server.send(
                JSON.stringify({
                    process: 'c-host',
                    type: 'stdout',
                    data:data
                }), function() {  });
            console.log('stdout: ' + data);
        });

        chost.stderr.on('data', function (data) {
            if(global.ws_server==undefined) return;
            global.ws_server.send(
            JSON.stringify({
                process: 'c-host',
                type: 'stderr',
                data:data
            }), function() { /* ignore errors */ });
            console.log('stderr: ' + data);
        });

        chost.on('close', function (code) {
            chost=undefined;
            if(global.ws_server==undefined) return;
            global.ws_server.send(
                JSON.stringify({
                    process: 'c-host',
                    type: 'status',
                    data: 'stopped'
                }), function() {  });
            console.log('c-host child process exited with code ' + code);
        });

        if(chost!=undefined){
            if(global.ws_server==undefined) return;
            global.ws_server.send(
                JSON.stringify({
                    process: 'c-host',
                    type: 'status',
                    data: 'running'
                }), function() {  });
        }
    };
};

exports.stophosts = function (db) {
    return function (req, res) {

        if(chost==undefined){
            return;
        }

        res.json(req.body);

        chost.kill();
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
            "nameOutGroup" : cell.nameOutGroup
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

