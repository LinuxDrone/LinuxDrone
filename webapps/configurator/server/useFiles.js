exports.metamodules = function (db) {
    return function (req, res) {
        db.get('modules_defs').find({}, {}, function (e, metaModules) {
            if(req.query.callback){
                res.send(req.query.callback + '(' + JSON.stringify(metaModules) + ')');
            }else{
                res.json(metaModules);
            }
        });
    };
};

exports.hello2 = function(request, response) {
    response.end("Hello, World!" + request.body.id);
};