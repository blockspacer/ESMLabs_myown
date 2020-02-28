var express = require('express');
var app = express();
var server = require('http').createServer(app);
var ws = require('ws');
var wss = new ws.Server({server});
var clients = new Map();
var image_processor = null;

wss.on('connection', function connection(wse, req) {
    wse.on('message', function(message) {
        var msg = JSON.parse(message);
        switch(msg.source) {
            case "ESM4DAP" :
                switch(msg.command) {
                    case "join" :
                        clients.set(msg.source_uuid, wse);
                        break;
                    case "leave" :
                        clients.delete(msg.source_uuid);
                        break;
                    case "offer" :
                        if(image_processor!=null) {
                            image_processor.send(JSON.stringify(msg));
                        }
                        break;
                }
                break;
            case "ESMImageProcessor" :
                switch(msg.command) {
                    case "join" :
                        image_processor = wse;
                        break;
                    case "leave" :
                        image_processor = null;
                        break;
                    case "answer" :
                        var target_wse = clients.get(msg.target_uuid);
                        target_wse.send(JSON.stringify(msg));
                        break;
                }                
                break;
        }
    });

    wse.on('error', function() {
        var uuid = null;
        clients.forEach(function(value, key) {
            if(value === wse) {
                uuid = key;
            }
        });
        if(uuid!=null) {
            clients.delete(uuid);
        }
        if(image_processor === wse)
            image_processor = null;
    });

    wse.on('close', function() {
        client.delete(wse);
        if(image_processor === wse)
            image_processor = null;
    });
})


server.listen(8080, function() {
    console.log('server is listening on 8080 port');
})