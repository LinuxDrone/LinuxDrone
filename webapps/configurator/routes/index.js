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
        var collection = db.get('modules_defs');
        collection.find({}, {}, function (e, docs) {
            res.json(docs);
        });
    };
};

exports.saveconfig = function (db) {
    return function (req, res) {
        //req.body.version = parseInt(req.body.version);
        //console.log(req.body);
        var collection = db.get('visual_configuration');

        collection.update({"name": req.body.name, "version": req.body.version}, req.body, {"upsert":true }, function (err,count){
            if(err)
            {
                res.send("There was a problem adding the information to the database.");
                return console.log(err);
            }
            console.log("Save configuration " + req.body.name + " v." +req.body.version + " - OK.");
        });

        var configuration = ConvertGraph2Configuration(JSON.parse(req.body.jsonGraph));
        configuration.version = req.body.version;
        configuration.name = req.body.name;

        var configurations = db.get('configuration');
        configurations.update({"name": req.body.name, "version": req.body.version}, configuration, {"upsert":true }, function (err,count){
            if(err)
            {
                res.send("There was a problem adding the information to the database.");
                return console.log(err);
            }
            console.log("Save LinuxDrone configuration " + req.body.name + " v." +req.body.version + " - OK.");
        });
        res.send("OK");
    };
};

exports.delconfig = function (db) {
    return function (req, res) {
        //req.body.version = parseInt(req.body.version);

        var collection = db.get('visual_configuration');
        collection.remove({"name":req.body.name, "version":req.body.version});

        var configurations = db.get('configuration');
        configurations.remove({"name":req.body.name, "version":req.body.version});
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
function ConvertGraph2Configuration(graph) {
    var config = {
        "type": "configuration",
        "modules": new Array(),
        "links": new Array()
    };

    graph.cells.forEach(function (cell) {
        if (cell.type == "devs.Model") {
            config.modules.push({
                "name": cell.attrs[".label"].text,
                /* Название instance модуля*/
                "instance": cell.attrs[".label"].text,
                /* Приоритет потока (задачи) xenomai */
                "task_priority": 80
            });
        }

        if (cell.type == "link") {
            config.links.push({
                "type": "pipe",
                "outInst": GetInstanceName(graph, cell.source.id),
                "inInst": GetInstanceName(graph, cell.target.id),
                "outPin": cell.source.port,
                "inPin": cell.target.port
            });
        }
    });

    //console.log(config);
    return config;
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

