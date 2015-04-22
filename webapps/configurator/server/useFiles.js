var CFG_FOLDER = "../../cfg";

var fs = require('fs');

var hostStatus = {
    status: 'stopped', // running
    schemaName: '',
    schemaVersion: ''
};


exports.gethoststatus = function (req, res) {
    if(req.body.callback){
        res.writeHead(200, {"Content-Type": "application/javascript"});
        res.write(req.body.callback + '(' + JSON.stringify(hostStatus) + ')');
    }else{
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

            if (ar[ar.length-1] == "json") {
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
    var record = JSON.parse(req.body.records);

    var cfgFileName = CFG_FOLDER + "/" + record.name + "." + record.version + ".json";
    fs.writeFile(cfgFileName, req.body.records, function(err) {
        if(err) {
            res.send({"success": false, "message": err});
            return console.log(err);
        }

        if (req.body.callback) {
            res.writeHead(200, {"Content-Type": "application/javascript"});
            res.write(req.body.callback + '({"success": true})');
        } else {
            res.writeHead(200, {"Content-Type": "application/json"});
            res.write('{"success": true}');
        }

        console.log("The file \""+cfgFileName+"\" was saved!");
    });
}

