

var dgram = require('dgram');
var socketio = require('socket.io');
var io;

exports.listen = function(server){
	io = socketio.listen(server);
	io.sockets.on('connection', function(socket){
		
		
	handleCommand(socket);
	});

};

function handleCommand(socket){
	

	socket.on('request', function(data){
		var errorTimer = setTimeout(function() {
			socket.emit("bberror", "No response from beaglebone. Is the program running?");
		}, 1000);
		// Info for connecting to the local process via UDP
		
		var HOST = '127.0.0.1';

		var buffer = new Buffer(data);
		if (data.charAt(0) == 0) {
			var PORT = 12345;
		} else {
			var PORT = 12346;
		}

		var client = dgram.createSocket('udp4');
		socket.emit('requestRecieved', "");
		client.send(buffer, 0, buffer.length, PORT, HOST, function(err, bytes) {
		    if (err) 
		    	throw err;
		    console.log('UDP message sent to ' + HOST +':'+ PORT);
		});
		
		client.on('listening', function () {
		    var address = client.address();
		    console.log('UDP Client: listening on ' + address.address + ":" + address.port);
		
		});
		// Handle an incoming message over the UDP from the local application.
		client.on('message', function (message, remote) {
		    console.log("UDP Client: message Rx" + remote.address + ':' + remote.port +' - ' + message);
		    
		    var reply = message.toString('utf8')
		    socket.emit('commandReply', reply);
		    
		    client.close();
		    clearTimeout(errorTimer);
		});
		client.on("UDP Client: close", function() {
		    console.log("closed");
		});
		client.on("UDP Client: error", function(err) {
		    console.log("error: ",err);
		});
		
	});
};