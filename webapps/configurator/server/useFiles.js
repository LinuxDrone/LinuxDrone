

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