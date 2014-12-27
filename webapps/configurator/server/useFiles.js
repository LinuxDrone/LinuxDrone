

exports.hello2 = function(request, response) {
    response.end("Hello, World!" + request.body.id);
};