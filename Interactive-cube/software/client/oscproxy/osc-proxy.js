var osc = require("osc");

var app = require('http').createServer(handler)
var io = require('socket.io')(app);
var fs = require('fs');

var agg_vector = [ 0, 0, 0,  0,  255, 0, 255,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ];
var last_vector = [ 0, 0, 0,  0,  255, 0, 255,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ];

function scaleVector(v) {
  return v;
    if (v <= -7) {
        return -lerp(Math.abs(v), 7,180, 0, 300);
    } else if (v >= 7) {
        return lerp(Math.abs(v), 7,180, 0, 300);
    }
    return 0.0;
}

app.listen(9000);

function handler (req, res) {
  fs.readFile(__dirname + '/index.html',
  function (err, data) {
    if (err) {
      res.writeHead(500);
      return res.end('Error loading index.html');
    }

    res.writeHead(200);
    res.end(data);
  });
}

io.on('connection', function (socket) {
  console.log('connected.');
    socket.emit('welcome', { hello: 'world' });
  // socket.on('my other event', function (data) {
  //   console.log(data);
  // });
});

// Create an osc.js UDP Port listening on port 57121.
var udpPort = new osc.UDPPort({
    localAddress: "192.168.1.100",
    localPort: 8002,
    // multicast: true
});

// Listen for incoming OSC bundles.
udpPort.on("bundle", function (d) {
    console.log("An OSC bundle just arrived!", d);
    //    io.emit('bundle', oscBundle);
});

// Listen for incoming OSC bundles.
udpPort.on("message", function (d) {
  // console.log('got message', d);
  io.emit('message', d);
});

// Open the socket.
udpPort.open();


function lerp(x, x0, x1, y0, y1) {
    var v = (x - x0) / (x1 - x0);
    v = Math.max(0, Math.min(1, v));
    v = (v * (y1 - y0)) + y0;
    return v;
}


setInterval(function() {
    for(var k=0; k<agg_vector.length; k++) {
        agg_vector[k] += (last_vector[k] - agg_vector[k]) * 0.95;
    }
    // console.log('sending state', agg_vector);
    io.emit('state', agg_vector);
}, 50);
