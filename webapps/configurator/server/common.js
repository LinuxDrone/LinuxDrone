exports.CFG_FOLDER = "../../cfg";

var BIN_FOLDER = "../../../x64/Debug"; // Windows
if(process.platform==='darwin'){
    BIN_FOLDER = "../../bin"; // Linux BBB
}




var fs = require('fs');
var exec = require('child_process').exec;


exports.requireUncached = function (module) {
    delete require.cache[require.resolve(module)];
    return require(module);
}

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

        this.meta = new Array();

        var self = this;

        fs.readdir(BIN_FOLDER, function (err, list) {
            if (err) {
                console.log(err);
                return;
            }

            var listFiles = new Array();
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
}

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


exports.runhosts = function (req, res) {

    if (hostStatus.status === 'running') {
        res.send({"success": false, message: "Host already running"});
        return;
    }

    fs.readdir(exports.CFG_FOLDER, function (err, list) {
        if (err) {
            console.log(err);
            return;
        }

        var cfgFiles = new Array();
        list.forEach(function (file) {
            // для начала проверим, что расширение файла .json
            var ar = file.split('.');

            if (ar[ar.length - 1] == "json") {
                var schema = exports.requireUncached(exports.CFG_FOLDER + '/' + file);
                if (schema.name === req.body.name && schema.version === req.body.version) {
                    console.log('Нашлась схема ' + schema.name + " " + schema.version);




                    if (req.body.callback) {
                        res.writeHead(200, {"Content-Type": "application/javascript"});
                        res.write(req.body.callback + '(' + JSON.stringify({"success": true}) + ')');
                    } else {
                        res.writeHead(200, {"Content-Type": "application/json"});
                        res.write(JSON.stringify({"success": true}));
                    }
                    res.end();
                }
            }
        });
    });

/*
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
            }), function () {
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
*/

};