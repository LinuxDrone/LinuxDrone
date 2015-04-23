var CFG_FOLDER = "../../cfg";

var fs = require('fs');

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

exports.getconfigs = function (req, res) {
    fs.readdir(CFG_FOLDER, function (err, list) {
        if (err) {
            console.log(err);
            return;
        }

        var cfgFiles = new Array();
        list.forEach(function (file) {
            // для начала проверим, что расширение файла .json
            var ar = file.split('.');

            if (ar[ar.length - 1] == "json") {
                var cfg = require(CFG_FOLDER + '/' + file);
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

        var cfgFileName = CFG_FOLDER + "/" + record.name + "." + record.version + ".json";
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

    var curCfgFileName = CFG_FOLDER + "/" + record.name + "." + record.version + ".json";
    var delCfgFileName = CFG_FOLDER + "/" + record.name + "." + record.version + "_" + getTimeStamp() + ".deleted";

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
