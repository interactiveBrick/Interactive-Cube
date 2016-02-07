

var osc = require('node-osc');


function Cube() {
  this.id = 'cubecube';
  this.listeners = [];
  this.timer = 0;
  this.queue = [];
  this.client = null;
  this.manager = null;
}


Cube.prototype.addListener = function(callback) {
  this.listeners.push(callback);
}

Cube.prototype._handleMessage = function(msg) {
  var prefix = '/' + this.id;

  if (msg[0].substring(0, prefix.length) != prefix)
    return;

  this.listeners.forEach(function(x) { x(msg); });
}


Cube.prototype.queuePopQueue = function() {
  clearTimeout(this.timer);
  this.timer = setTimeout(this.popQueue.bind(this), 3);
}

Cube.prototype.popQueue = function() {
  if (this.queue.length < 1) {
    return;
  }

  var _this = this;
  var item = this.queue.shift();
  // console.log('OSC Sending:', item);
  this.client.send(item[0], item[1], item[2], item[3], item[4], item[5], item[6], item[7], item[8], item[9], item[10], item[11], item[12], function() {
    // client.kill();
    _this.queuePopQueue();
  });
}


Cube.prototype.connect = function(hostname, port) {
  this.client = new osc.Client(hostname, port);
  this.queuePopQueue();
}


Cube.prototype.setLeds = function(leds) {

  var bytes = [0,0,0,0, 0,0,0,0, 0,0,0,0];

  for(var s=0; s<6; s++) {
    for(var j=0; j<8; j++) {
      if (leds[s*16+0+j])
        bytes[s*2+0] |= (1 << j);
      if (leds[s*16+8+j])
        bytes[s*2+1] |= (1 << j);
    }
  }

  for(var j=0; j<4; j++) {
    var line = '';
    for(var s=0; s<6; s++) {
      for(var i=0; i<4; i++) {
        var o = s * 16 + j * 4 + i;
        line += leds[o] ? '#' : '.';
      }
      line += ' ';
    }
    console.log('> ' + line);
  }
  console.log('');

  var msg = [ '/' + this.id + '/leds',
    bytes[0], bytes[1], bytes[2], bytes[3],
    bytes[4], bytes[5], bytes[6], bytes[7],
    bytes[8], bytes[9], bytes[10], bytes[11] ];

  this.queue.push(msg);

  this.io.sockets.emit('display', { leds: leds });

  this.queuePopQueue();
}

Cube.prototype.startDebugWebServer = function(port) {
  port = port || 3000;

  //
  // Set up mock UI webserver
  //

  var _this = this;

  var app = require('express')();
  var http = require('http').Server(app);
  this.io = require('socket.io')(http);

  app.get('/', function(req, res) {
    res.sendfile(__dirname + '/debug.html');
  });

  this.io.on('connection', function(socket){
    console.log('a user connected');
    socket.on('chat message', function(msg){
      console.log('message: ' + msg);
    });
    socket.on('buttondown', function(msg){
      console.log('buttondown:', msg);
      _this.manager._emit(['/' + _this.id + '/btn', msg.index, 1]);
    });
    socket.on('buttonup', function(msg){
      console.log('buttonup:', msg);
      _this.manager._emit(['/' + _this.id + '/btn', msg.index, 0]);
    });
    socket.on('disconnect', function(){
      console.log('user disconnected');
    });
  });

  http.listen(port, function() {
    console.log('listening on *:' + port);
  });
}




function CubeManager() {
  this.listeners = [];
  this.cubes = [];
}

CubeManager.prototype._emit = function(msg) {
  this.listeners.forEach(function(x) { x(msg); });
}

CubeManager.prototype.listen = function(port) {
  var _this = this;
  var oscServer = new osc.Server(3333, '0.0.0.0');

  oscServer.on("message", function (msg, rinfo) {
    // console.log("CubeManager: Got message", msg);
    _this.emit(msg);
  });
}

CubeManager.prototype.getCube = function(cubename) {
  var cube = new Cube();
  cube.id = cubename;
  cube.manager = this;
  this.addListener(cube._handleMessage.bind(cube));
  this.cubes.push(cube);
  return cube;
}

CubeManager.prototype.addListener = function(callback) {
  this.listeners.push(callback);
}


// exports.Cube = Cube;
exports.CubeManager = CubeManager;

