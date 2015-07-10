var fs = require('fs');

var common = require("./common");


exports.getconfigs = function (req, res) {
    fs.readdir(common.CFG_FOLDER, function (err, list) {
        if (err) {
            console.log(err);
            return;
        }

        var cfgFiles = new Array();
        list.forEach(function (file) {
            // для начала проверим, что расширение файла .json
            var ar = file.split('.');

            if (ar[ar.length - 1] == "json") {
                var cfg = common.requireUncached(common.CFG_FOLDER + '/' + file);
                cfgFiles.push(cfg);
            }
        });

        if (req.body.callback) {
            res.writeHead(200, {"Content-Type": "application/javascript"});
            res.write(req.body.callback + '(' + JSON.stringify(cfgFiles) + ')');
        } else {
            res.writeHead(200, {"Content-Type": "application/json"});
            res.write(JSON.stringify(cfgFiles));
        }
        res.end();
    });
};

exports.saveconfig = function (req, res) {
    var ar;
    if ( Array.isArray(req.body.records)) {
        ar = req.body.records;
    } else {
        ar = new Array();
        ar.push(req.body.records);
    }

    ar.forEach(function (item, i, arr) {
        var record = JSON.parse(item);

        var cfgFileName = common.CFG_FOLDER + "/" + record.name + "." + record.version + ".json";
        fs.writeFile(cfgFileName, JSON.stringify(record), function (err) {
            if (err) {
                res.send({"success": false, "message": err});
                res.end();
                return console.log(err);
            }
            console.log("The file \"" + cfgFileName + "\" was saved!");
        });
    });

    if (req.body.callback) {
        res.writeHead(200, {"Content-Type": "application/javascript"});
        res.write(req.body.callback + '({"success": true})');
    } else {
        res.writeHead(200, {"Content-Type": "application/json"});
        res.write('{"success": true}');
    }
    res.end();
}

function getTimeStamp() {
    var now = new Date();
    return ((now.getMonth() + 1) + '-' + (now.getDate()) + '-' + now.getFullYear() + "_" + now.getHours() + '-' + ((now.getMinutes() < 10) ? ("0" + now.getMinutes()) : (now.getMinutes())) + '-' + ((now.getSeconds() < 10) ? ("0" + now.getSeconds()) : (now.getSeconds())));
}

exports.delconfig = function (req, res) {
    var record = JSON.parse(req.body.records);

    var curCfgFileName = common.CFG_FOLDER + "/" + record.name + "." + record.version + ".json";
    var delCfgFileName = common.CFG_FOLDER + "/" + record.name + "." + record.version + "_" + getTimeStamp() + ".deleted";

    fs.rename(curCfgFileName, delCfgFileName, function(err) {
        if (err) {
            res.send({"success": false, "message": err});
            res.end();
            return console.log(err);
        }
        console.log("The file \"" + curCfgFileName + "\" was renamed to " + delCfgFileName);

        if (req.body.callback) {
            res.writeHead(200, {"Content-Type": "application/javascript"});
            res.write(req.body.callback + '({"success": true})');
        } else {
            res.writeHead(200, {"Content-Type": "application/json"});
            res.write('{"success": true}');
        }
        res.end();
    });
};

exports.getconfig = function (req, res) {
    fs.readdir(common.CFG_FOLDER, function (err, list) {
        if (err) {
            console.log(err);
            return;
        }
        list.forEach(function (file) {
            // для начала проверим, что расширение файла .json
            var ar = file.split('.');

            if (ar[ar.length - 1] == "json") {
                var cfg = common.requireUncached(common.CFG_FOLDER + '/' + file);
                if(cfg["_id"]===req.params.id){
                    res.setHeader("Content-Type", "application/json");
                    res.setHeader("Content-Disposition", "attachment;filename='" + cfg.name + "." + cfg.version + ".json'");
                    res.write(JSON.stringify(cfg));
                    res.end();
                    return;
                }
            }
        });
    });
};

// Возвращает шелл скрипт, для ручного запуска инстансов в командной строке
exports.getscript = function (req, res) {
    fs.readdir(common.CFG_FOLDER, function (err, list) {
        if (err) {
            console.log(err);
            return;
        }
        list.forEach(function (file) {
            // для начала проверим, что расширение файла .json
            var ar = file.split('.');

            if (ar[ar.length - 1] == "json") {
                var schema = common.requireUncached(common.CFG_FOLDER + '/' + file);
                if(schema["_id"]===req.params.id){
                    common.GetConfiguration(schema, function (configuration) {
                        res.setHeader("Content-Type", "application/json");
                        res.setHeader("Content-Disposition", "attachment;filename='run_" + schema.name + "." + schema.version + ".cmd'");

                        configuration.modules.forEach(function (instance) {
                            //console.log(instance.instance);
                            var instanceCmdParams = common.MakeInstanceCommandParams(instance, configuration);

                            var cmdName = instance.name + ".mod";
                            cmdName += ".exe";

                            instanceCmdParams.forEach(function (param) {
                                cmdName += " " + param;
                            });

                            res.write(cmdName + "\r\n");
                        });
                        res.end();
                    });
                }
            }
        });
    });
};
