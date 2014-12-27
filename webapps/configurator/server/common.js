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
    res.write('[');


    fs.readdir(BIN_FOLDER, function (err, list) {
        if (err) {
            console.log(err);
            return;
        }

        // Рекурсивная функция. Запускает по очереди модйли перечисленные в массиве arr.
        // Возвращаемый результат (определение модуля в JSON) пишет в выходной поток http ответа
        var executeFile = function (arr, recursive, callback_end, was_previos) {
            if (arr.length > 0) {
                var fullFile = arr.pop();
                console.log(fullFile);

                exec(fullFile + ' --module-definition',
                    function (error, stdout, stderr) {
                        if (error !== null) {
                            console.log('exec error: ' + error);
                            return;
                        }
                        if (was_previos) {
                            res.write(',');
                        }
                        //console.log('stdout: ' + stdout);
                        // TODO: Проверять валидность возвращаемого JSON
                        res.write(stdout);

                        recursive(arr, recursive, callback_end, true);
                    });
            } else {
                callback_end();
            }
        }

        var listFiles = new Array();
        list.forEach(function (file) {
            // для начала проверим, что расширение файла .mod
            var ext = file.substr(file.length-4, 4);
            if(ext==".mod"){
                listFiles.push(BIN_FOLDER + '/' + file);
            }
        });

        executeFile(listFiles, executeFile, function () {
            res.write(']');
            if (req.body.callback) {
                res.write(')');
            }
            res.end();
        }, false);
    });


};
