
var url = 'ws://192.168.1.150:7681/';

// url = 'ws://192.168.1.27:7681/';
// url = 'ws://10.10.16.182:7681/';

var frame = 0;
var cols = 32 * 5;
var rows = 32;

var ctx;
var socket;

var down = 0;
var last_x = 0, last_y = 0;

var prepareFrameCallback;
var framebuffer = [];
for (var i=0; i<rows*cols*3; i++) framebuffer.push(0);//Math.floor(Math.random() * 255));

function ev_mousedown (ev) {
	down = 1;
}

function ev_mouseup(ev) {
	down = 0;
}

function blurb(ox,oy) {
	var o = (oy * cols + ox) * 3;
	framebuffer[o + 0] = Math.floor(Math.random() * 255);
	framebuffer[o + 1] = Math.floor(Math.random() * 255);
	framebuffer[o + 2] = Math.floor(Math.random() * 255);
}

function ev_mousemove (ev) {
	var x = ev.offsetX;
	var y = ev.offsetY;

	if (!down)
		return;

	var ox = Math.floor(x / 8);
	var oy = Math.floor(y / 8);

	for(var dx=-3; dx<=3; dx++)
		for(var dy=-3; dy<=3; dy++)
			blurb(ox+dx, oy+dy);

	last_x = x;
	last_y = y;
}

function queueFrame(delay) {
	setTimeout(sendFrame, delay || 0);
}

var outframe = 0;

var _gamma = function(r) {
	return (r * r) / 800;
}

function updateCanvas() {
	for(var j=0; j<rows; j++) {
		for(var i=0; i<cols; i++) {
			var o = (j * cols + i) * 3;
			var x = i * 8;
			var y = j * 8;
			ctx.fillStyle = 'rgb(' + Math.floor(framebuffer[o+0])+','+Math.floor(framebuffer[o+1])+','+Math.floor(framebuffer[o+2])+')';
			ctx.fillRect(x,y,8,8);
		}
	}
}

var scan = 0;

var scanorder = [];
for(var i=0; i<rows; i++) {
	scanorder.push(i);
}

for(var i=0; i<rows; i++) {
	var i1 = Math.floor(Math.random() * rows);
	var i2 = Math.floor(Math.random() * rows);
	var r1 = scanorder[i1];
	var r2 = scanorder[i2];
	scanorder[i1] = r2;
	scanorder[i2] = r1;
}

var queueLines = 1;
var queueInterval = 40;

function RLEencode(input) {
    var encoding = [];
    var prev, count, i;
    for (count = 1, prev = input[0], i = 1; i < input.length; i++) {
        if (input[i] != prev || count > 250) {
            encoding.push([count, prev]);
            count = 1;
            prev = input[i];
        }
        else
            count ++;
    }
    encoding.push([count, prev]);
    return encoding;
}

var last_dummydata = '';
function sendFrame() {
	prepareFrameCallback(function() {

		//
		// full frames
		//
		var dummydata = [];
		var dummydata2 = [];

		var offset = 0;// (frame % 8) * 32;
		var perupdate = 32;

	//	dummydata.push('R');
	//	dummydata.push(String.fromCharCode(0 + offset));

		for(var j=0; j<perupdate; j++) {
			for(var i=0; i<cols; i++) {
				var o = ((j + offset) * cols + i) * 3;
				var _r = Math.round(framebuffer[o + 0] * 4 / 256);
				var _g = Math.round(framebuffer[o + 1] * 4 / 256);
				var _b = Math.round(framebuffer[o + 2] * 2 / 256);

//				if (_r < 0) _r = 0; if (_r > 255) _r = 255;
//				if (_g < 0) _g = 0; if (_g > 255) _g = 255;
//				if (_b < 0) _b = 0; if (_b > 255) _b = 255;

				dummydata.push(_r);// Math.floor(_r / 1));
				dummydata.push(_g);//Math.floor(_g / 1));
				dummydata.push(_b);//7Math.floor(_b / 1));


				// _r = (_r) & 7;
				//n	_g = (_g >> 3) & 7;
				//	_b = (_b >> 6) & 3;

				dummydata2.push((_b << 6) + (_g << 3) + (_r));
			}
		}

		/*
		var LL = 50 + Math.floor(Math.random() * 100);
		var RR = String.fromCharCode(0 + Math.floor(Math.random() * 255));

		for(var i=0; i<LL; i++) {
			dummydata[i] = 100;
		}

		for(var i=0; i<LL; i++) {
			dummydata[LL+i] = 0;
		}
		*/
		/*
		dummydata[2] = String.fromCharCode(0 + Math.floor(Math.random() * 255));
		dummydata[3] = String.fromCharCode(0 + Math.floor(Math.random() * 255));
		dummydata[4] = String.fromCharCode(0 + Math.floor(Math.random() * 255));
		*/
		// dummydata = dummydata.join('');


		var encoded = RLEencode(dummydata2);

		var encodeddata = [];
		var ooo = 0;
		encodeddata.push("r".charCodeAt(0));
		encodeddata.push(offset);
		encoded.forEach(function(x) {
			encodeddata.push(x[0]);
			encodeddata.push(x[1]);
			// encodeddata.push(String.fromCharCode(x[1]))
		});

//		console.log(encodeddata[0],	 encodeddata[1], encodeddata[2], encodeddata[3], encodeddata[4]);

//		console.log('raw length', dummydata.length, encodeddata.length);

		// dummydata = 'B' + String.fromCharCode(0 + offset) + window.btoa(dummydata);

		dummydata = window.btoa(String.fromCharCode.apply(null, new Uint8Array(encodeddata)));

//		console.log('raw length', dummydata.length, dummydata);

	if (dummydata != last_dummydata) {
		last_dummydata = dummydata;
		socket.send(dummydata);
	}



//		console.log('sending data', dummydata.length);

		outframe ++;

		updateCanvas();
		queueFrame(queueInterval);
	});
}

function connectWebsocket() {
	socket = new WebSocket(url, "ledpanel-frame");

	socket.onopen = function() {
		document.getElementById("wsdi_status").textContent = " websocket connection opened ";
		queueFrame(200);
	}

	socket.onmessage =function got_packet(msg) {
		document.getElementById("number").textContent = msg.data + "\n";
	}

	socket.onclose = function(){
		document.getElementById("wsdi_status").textContent = " websocket connection CLOSED ";
	}
}

function reset() {
	socket.send("reset\n");
}

// function waitNextFrame(callback) {
// }

function updateFrame(callback) {
	callback();
}

// initCanvas();
// connectWebsocket();


function initLedpanel(prepareFrame) {
	prepareFrameCallback = prepareFrame;
	initCanvas();
	connectWebsocket();
}
