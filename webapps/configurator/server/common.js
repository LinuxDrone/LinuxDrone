var BIN_FOLDER = "../../bin";

var fs = require('fs');
var exec = require('child_process').exec;


// Получает определения модулей, запуская модули с параметром --module-definition
// Полученные определения возвращает в виде массива JSON
exports.metamodules = function (req, res) {
    if (req.body.callback) {
        res.writeHead(200, {"Content-Type": "application/javascript"});
        res.write(req.body.callback + '(');
    } else {
        res.writeHead(200, {"Content-Type": "application/json"});
    }

    exports.MetaOfModules.get(function(meta){
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
    executeFile : function (ar_res, files, recursive, callback_end) {
        if (files.length > 0) {
            var fullFile = files.pop();

            process.stdout.write("execute " + fullFile + " --module-definition");

            exec(fullFile + ' --module-definition',
                function (error, stdout, stderr) {
                    if (error !== null) {
                        console.log('exec error: ' + error);
                        return;
                    }else{
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
    get : function (callback) {
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
                // для начала проверим, что расширение файла .mod
                var ext = file.substr(file.length - 4, 4);
                if (ext == ".mod") {
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
