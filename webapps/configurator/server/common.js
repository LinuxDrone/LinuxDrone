var os = require('os');

exports.CFG_FOLDER = "../../cfg";

var BIN_FOLDER = "../../bin"; // Unix
if(os.type()=="Windows_NT"){
    BIN_FOLDER = "../../../x64/Debug"; // Windows
}

var commonModuleParams = require('../client/ModulesCommonParams.def.js');
var _ = require('underscore');
var fs = require('fs');
var exec = require('child_process').exec;
var spawn = require('child_process').spawn;


// Приведение значения к типу
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

// Выгреюает информацию о инстансе из визуальной конфигурации и раскладывает по полочкам
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
        if (modulesParams && modulesParams[module.instance] && modulesParams[module.instance].common) { //&& modulesParams[module.instance]
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
        } else {
            console.log("For instance " + module.instance + ". Not found common parameters");
            return;
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
            module.blocksConfig = {"blocks": [], "links": []};

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

// Конвертирует визуальное представление графа в конфигурацию модулей принятую в linuxdrone
function ConvertGraph2Configuration(graph, modulesParams, metaModules) {
    var config = {
        "type": "configuration",
        "modules": [],
        "links": []
    };

    graph.cells.forEach(function (cell) {
        ConvertVisualCell(graph, config.modules, config, cell, modulesParams, metaModules);
    });

    //console.log(config);
    return config;
}

// Генерит строку параметров командной строки, для запуска инстанса модуля
exports.MakeInstanceCommandParams = function (instance, configuration) {
    var res = [];
    res.push("--name=" + instance.instance);

    var addrs = instance.instance.split(":");
    if (addrs.length > 1) {
        var port = addrs[1];
        res.push("--port=" + port);
    }

    res.push("--rt-priority=" + instance["rt-priority"]);
    res.push("--main-task-period=" + instance["main-task-period"]);
    res.push("--transfer-task-period=" + instance["transfer-task-period"]);


    Object.keys(instance.params).forEach(function (paramName) {
        var paramValue = instance.params[paramName];

        var normParamName = paramName.replace(/\ /g, "_").replace(/\+/g, "");

        if(paramValue.constructor.name == "String"){
            res.push("--"+normParamName+"=\"" + paramValue +"\"");
        }else{
            res.push("--"+normParamName+"=" + paramValue);
        }
    });


    configuration.links.forEach(function (link) {
        if (link.inInst == instance.instance && link.type == "memory") {
            res.push("--in-link=" + link.outInst + "@" + link.nameOutGroup + "@" + link.outPin + "#" + link.inPin);
        }
        if (link.outInst == instance.instance && link.type == "queue") {
            res.push("--out-link=" + link.outPin + "#" + link.inInst + "@" + link.inPin);
        }
    });

    return res;
};


exports.requireUncached = function (module) {
    delete require.cache[require.resolve(module)];
    return require(module);
};

// Получает определения модулей, запуская модули с параметром --module-definition
// Полученные определения возвращает в виде массива JSON
exports.metamodules = function (req, res) {
    if (req.body.callback) {
        res.writeHead(200, {"Content-Type": "application/javascript"});
        res.write(req.body.callback + '(');
    } else {
        res.writeHead(200, {"Content-Type": "application/json"});
    }

    exports.MetaOfModules.get(function (meta) {
        res.write(JSON.stringify(meta));

        if (req.body.callback) {
            res.write(')');
        }
        res.end();
    });
};

// Объект, содержащий метод get, возвращающий массив с определениями модулей
exports.MetaOfModules = {
    meta: undefined,

    // Рекурсивная функция. Запускает по очереди модйли перечисленные в массиве arr.
    // Возвращаемый результат (определение модуля в JSON) пишет в выходной поток http ответа
    executeFile: function (ar_res, files, recursive, callback_end) {
        if (files.length > 0) {
            var fullFile = "\"" + files.pop() + "\"";

            process.stdout.write("execute " + fullFile + " --module-definition");

            exec(fullFile + ' --module-definition',
                function (error, stdout, stderr) {
                    if (error !== null) {
                        console.log('exec error: ' + error);
                        return;
                    } else {
                        console.log('\tOK');
                    }

                    ar_res.push(JSON.parse(stdout));

                    recursive(ar_res, files, recursive, callback_end);
                });
        } else {
            callback_end();
        }
    },

    // Возвращает в калбеке массив с определениями модулей
    get: function (callback) {
        if (this.meta != undefined) {
            callback(this.meta);
            return;
        }

        this.meta = [];

        var self = this;

        fs.readdir(BIN_FOLDER, function (err, list) {
            if (err) {
                console.log(err);
                return;
            }

            var listFiles = [];
            list.forEach(function (file) {
                // для начала проверим, что расширение файла .mod или .mod.exe
                var ar = file.split('.');

                if (ar[ar.length - 1] == "mod" || (ar[ar.length - 2] == "mod" && ar[ar.length - 1] == "exe")) {
                    listFiles.push(BIN_FOLDER + '/' + file);
                }
            });

            // Вызовем рекурсивную функцию, передав массив с именами исполнимых файлов модулей
            self.executeFile(self.meta, listFiles, self.executeFile, function () {
                callback(self.meta);
            });
        });
    }
};

var hostStatus = {
    status: 'stopped', // running
    schemaName: '',
    schemaVersion: ''
};


exports.gethoststatus = function (req, res) {
    if (req.body.callback) {
        res.writeHead(200, {"Content-Type": "application/javascript"});
        res.write(req.body.callback + '(' + JSON.stringify(hostStatus) + ')');
    } else {
        res.writeHead(200, {"Content-Type": "application/json"});
        res.write(JSON.stringify(hostStatus));
    }
    res.end();
};

var processes = {};

// В калбэк, в качестве параметра, передает конфигурацию, созданную из графической схемы
exports.GetConfiguration = function(schema, callback) {
    exports.MetaOfModules.get(function (metaModules) {
        // Получаем описания модулей и вызываем функцию, передавай ей граф объектов из графической схемы и настроечные параметры модулей.
        callback(ConvertGraph2Configuration(JSON.parse(schema.jsonGraph), schema.modulesParams, metaModules));
    });
};

exports.runhosts = function (req, res) {
    if (hostStatus.status === 'running') {
        if (req.body.callback) {
            res.writeHead(200, {"Content-Type": "application/javascript"});
            res.write(req.body.callback + '(' + JSON.stringify({
                "success": false,
                message: "Host already running"
            }) + ')');
        } else {
            res.writeHead(200, {"Content-Type": "application/json"});
            res.write(JSON.stringify({"success": false, message: "Host already running"}));
        }
        res.end();
        return;
    }

    var fileName = exports.CFG_FOLDER + "/" + req.body.name + "." + req.body.version + ".json";
    var schema = exports.requireUncached(fileName);
    if (schema.name === req.body.name && schema.version === req.body.version) {
        // Теперь надо вытащить из схемы все инстансы, сформировать для каждого из них параметры командной строки запустить каждый в отдельном процессе.
        exports.GetConfiguration(schema, function (configuration) {
            configuration.modules.forEach(function (instance) {
                //console.log(instance.instance);
                var instanceCmdParams = exports.MakeInstanceCommandParams(instance, configuration);

                var cmdName = instance.name + ".mod";
                if(os.type()=="Windows_NT"){
                    cmdName += ".exe";
                }

                var new_process;
                try {
                    new_process = spawn(BIN_FOLDER + "/" + cmdName, instanceCmdParams, {env: process.env});
                } catch (e) {
                    console.log(e);
                    res.send({"success": false, "message": e});
                    res.end();
                    return;
                }
                processes[instance.instance] = new_process;

                hostStatus.status = 'running';
                hostStatus.schemaName = schema.name;
                hostStatus.schemaVersion = schema.version;

                new_process.stdout.on('data', function (data) {
                    if (global.ws_server == undefined) return;
                    //console.log("prc=" + instance.instance);
                    global.ws_server.send(
                        JSON.stringify({
                            process: instance.instance,
                            type: 'stdout',
                            data: data
                        }), function () {
                        });
                    console.log('stdout: ' + data);
                });

                new_process.stderr.on('data', function (data) {
                    if (global.ws_server == undefined) return;
                    global.ws_server.send(
                        JSON.stringify({
                            process: instance.instance,
                            type: 'stderr',
                            data: data
                        }), function () {
                        });
                    console.log('stderr: ' + data);
                });

                new_process.on('close', function (code) {
                    hostStatus.status = 'stopped';
                    if (global.ws_server != undefined) {
                        global.ws_server.send(JSON.stringify(hostStatus), function () {
                        });
                    }
                    hostStatus.schemaName = '';
                    hostStatus.schemaVersion = '';

                    console.log('close c-host child process exited with code ' + code);

                    global.ws_server.send(
                        JSON.stringify({
                            process: instance.instance,
                            type: 'stdout',
                            data: new Buffer(instance.instance + ' child process exited with code ' + code)
                        }), function () {
                        });
                });

                new_process.on('error', function (code) {
                    hostStatus.status = 'stopped';
                    if (global.ws_server != undefined) {
                        global.ws_server.send(JSON.stringify(hostStatus), function () {
                        });
                    }
                    hostStatus.schemaName = '';
                    hostStatus.schemaVersion = '';

                    console.log('error c-host child process exited with code ' + code);
                });

                if (global.ws_server != undefined) {
                    global.ws_server.send(JSON.stringify(hostStatus), function () {
                    });
                }
            });

            if (req.body.callback) {
                res.writeHead(200, {"Content-Type": "application/javascript"});
                res.write(req.body.callback + '(' + JSON.stringify({"success": true}) + ')');
            } else {
                res.writeHead(200, {"Content-Type": "application/json"});
                res.write(JSON.stringify({"success": true}));
            }
            res.end();
        });
    } else {
        console.log("Обнаружено несоответсвие имени и версии схемы указанные в названии файла, имени и версии в содержимом файла схемы " + fileName);
    }
};


exports.stophosts = function (req, res) {

    if (Object.keys(processes).length == 0) {
        res.send({"success": false, message: "Host already stopped"});
        res.end();
        return;
    }

    Object.keys(processes).forEach(function (key) {
        stopped_process = processes[key];
        try {
            stopped_process.kill();
        } catch (e) {
            console.log(e);
            res.send({"success": false, "message": e});
            res.end();

        }
    });

    processes = {};


    hostStatus.status = 'stopped';
    if (global.ws_server != undefined) {
        global.ws_server.send(JSON.stringify(hostStatus), function () {
        });
    }
    hostStatus.schemaName = '';
    hostStatus.schemaVersion = '';


    if (req.body.callback) {
        res.writeHead(200, {"Content-Type": "application/javascript"});
        res.write(req.body.callback + '(' + JSON.stringify({"success": true}) + ')');
    } else {
        res.writeHead(200, {"Content-Type": "application/json"});
        res.write(JSON.stringify({"success": true}));
    }
    res.end();
};