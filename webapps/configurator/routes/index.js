var _ = require('underscore');
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

exports.saveconfig = function (db) {
    return function (req, res) {
        //req.body.version = parseInt(req.body.version);
        //console.log(req.body);
        var collection = db.get('visual_configuration');

        collection.update({"name": req.body.name, "version": req.body.version}, req.body, {"upsert": true }, function (err, count) {
            if (err) {
                res.send("There was a problem adding the information to the database.");
                return console.log(err);
            }
            console.log("Save configuration " + req.body.name + " v." + req.body.version + " - OK.");
        });

        db.get('modules_defs').find({}, {}, function (e, metaModules) {
            var configuration = ConvertGraph2Configuration(JSON.parse(req.body.jsonGraph), req.body.modulesParams, metaModules);
            configuration.version = req.body.version;
            configuration.name = req.body.name;

            var configurations = db.get('configuration');
            configurations.update({"name": req.body.name, "version": req.body.version}, configuration, {"upsert": true }, function (err, count) {
                if (err) {
                    res.send("There was a problem adding the information to the database.");
                    return console.log(err);
                }
                console.log("Save LinuxDrone configuration " + req.body.name + " v." + req.body.version + " - OK.");
            });
            res.send("OK");
        });
    };
};

exports.delconfig = function (db) {
    return function (req, res) {
        //req.body.version = parseInt(req.body.version);

        var collection = db.get('visual_configuration');
        collection.remove({"name": req.body.name, "version": req.body.version});

        var configurations = db.get('configuration');
        configurations.remove({"name": req.body.name, "version": req.body.version});

        res.send("OK");
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
        case "number":
            value = Number(value);
            break;
        case "boolean":
            if (_.isString(value) && value === "false") {
                value = false;
            }
            else {
                value = Boolean(value);
            }
            break;
        case "string":
            value = String(value);
            break;
        default:
            return undefined;
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
        if (modulesParams[module.instance].common) { //modulesParams[module.instance] &&
            var commonParams = modulesParams[module.instance].common;
            Object.keys(commonParams).forEach(function (paramName) {
                var metaParam = _.find(commonModuleParams.commonModuleParamsDefinition, function (meta) {
                    return meta.name == paramName;
                });
                if (!metaParam) {
                    console.log("Not found metadata for parameter '" + paramName + "'  in common definition for modules ");
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

