var CubeManager = require('../nodelib/cube/index.js').CubeManager;

//
// Set up cube connection
//

var cubemanager = new CubeManager();
cubemanager.listen(3333);

var cube = cubemanager.getCube('cube18FE');
cube.connect('192.168.1.41', 3333);

cube.startDebugWebServer(3000);



var D_UP = 0;
var D_RT = 1;
var D_DN = 2;
var D_LT = 3;

var S_FT = 0;
var S_RT = 1;
var S_BK = 2;
var S_LT = 3;
var S_TP = 4;
var S_BT = 5;


var G = {
  state: 0,

  side: S_RT,
  x: 2,
  y: 2,
  dir: D_DN, // 0=up, 1=right...

  tside: S_BT,
  tx: 1,
  ty: 1,

  aside: S_BT,
  ax: 1,
  ay: 1,

  lasttick: 0,
  starttime: 0,
  time: 0,

  length: 4,
  history: []
};

function stepGame() {
  console.log('stepGame');
  console.log('inp state', G.side, G.x, G.y, G.dir);

  G.history.push({
    x: G.x,
    y: G.y,
    dir: G.dir,
    side: G.side
  });

  if (G.history.length > G.length) {
    G.history.splice(0, 1);
  }

  switch(G.dir) {
    case D_LT: {
      G.x -= 1;
      if (G.x < 0) {
        console.log('Wrap left side.');
        switch(G.side) {
          case S_FT: G.side = S_LT; G.dir = D_LT; G.x = 3;       G.y = G.y; break; // OK
          case S_RT: G.side = S_FT; G.dir = D_LT; G.x = 3;       G.y = G.y; break; // OK
          case S_BK: G.side = S_RT; G.dir = D_LT; G.x = 3;       G.y = G.y; break; // OK
          case S_LT: G.side = S_BK; G.dir = D_LT; G.x = 3;       G.y = G.y; break; // OK
          case S_TP: G.side = S_LT; G.dir = D_DN; G.x =     G.y; G.y = 0;   break; // OK
          case S_BT: G.side = S_LT; G.dir = D_UP; G.x = 3 - G.y; G.y = 3;   break; // OK
        }
      }
      break;
    }
    case D_RT: {
      G.x += 1;
      if (G.x > 3) {
        console.log('Wrap to the right.');
        switch(G.side) {
          case S_FT: G.side = S_RT; G.dir = D_RT; G.x = 0;       G.y = G.y; break; // OK
          case S_RT: G.side = S_BK; G.dir = D_RT; G.x = 0;       G.y = G.y; break; // OK
          case S_BK: G.side = S_LT; G.dir = D_RT; G.x = 0;       G.y = G.y; break; // OK
          case S_LT: G.side = S_FT; G.dir = D_RT; G.x = 0;       G.y = G.y; break; // OK
          case S_TP: G.side = S_RT; G.dir = D_DN; G.x = 3 - G.y; G.y = 0;   break; // OK
          case S_BT: G.side = S_RT; G.dir = D_UP; G.x =     G.y; G.y = 3;   break; // OK
        }
      }
      break;
    }
    case D_UP: {
      G.y -= 1;
      if (G.y < 0) {
        console.log('Wrap on top.');
        switch(G.side) {
          case S_FT: G.side = S_TP; G.dir = D_UP; G.x =     G.x; G.y = 3; break; // OK
          case S_RT: G.side = S_TP; G.dir = D_LT; G.y = 3 - G.x; G.x = 3; break; // OK
          case S_BK: G.side = S_TP; G.dir = D_DN; G.x = 3 - G.x; G.y = 0; break; // OK
          case S_LT: G.side = S_TP; G.dir = D_RT; G.y =     G.x; G.x = 0; break; // OK
          case S_TP: G.side = S_BK; G.dir = D_DN; G.x = 3 - G.x; G.y = 0; break; // OK
          case S_BT: G.side = S_FT; G.dir = D_UP; G.x =     G.x; G.y = 3; break; // OK
        }
      }
      break;
    }
    case D_DN: {
      G.y += 1;
      if (G.y > 3) {
        console.log('Wrap on bottom.');
        switch(G.side) {
          case S_FT: G.side = S_BT; G.dir = D_DN; G.x =     G.x; G.y = 0; break; // OK
          case S_RT: G.side = S_BT; G.dir = D_LT; G.y =     G.x; G.x = 3; break; // OK
          case S_BK: G.side = S_BT; G.dir = D_UP; G.x = 3 - G.x; G.y = 3; break; // OK
          case S_LT: G.side = S_BT; G.dir = D_RT; G.y = 3 - G.x; G.x = 0; break; // OK
          case S_TP: G.side = S_FT; G.dir = D_DN; G.x =     G.x; G.y = 0; break; // OK
          case S_BT: G.side = S_BK; G.dir = D_UP; G.x = 3 - G.x; G.y = 3; break; // OK
        }
      }
      break;
    }
  }

  if (G.x == G.ax && G.y == G.ay && G.side == G.aside) {
    G.length ++;
    G.ax = Math.floor(Math.random() * 4);
    G.ax = Math.floor(Math.random() * 4);
    G.aside = Math.floor(Math.random() * 6);
  }

  console.log('out state', G.side, G.x, G.y, G.dir);
}


cube.addListener(function(msg) {
  if (msg[0] == '/' + cube.id + '/btn') {
    if (msg[2] == 1) {
      var n = msg[1];

      G.tside = (n >> 4);
      G.tx = (n % 16) % 4;
      G.ty = (n % 16) >> 2;


      if (G.tside == G.side) {
        // same side... check x/y
        if (G.tx < G.x) {
          G.dir = D_LT;
        } else if (G.tx > G.x) {
          G.dir = D_RT;
        } else if (G.ty < G.y) {
          G.dir = D_UP;
        } else if (G.ty > G.y) {
          G.dir = D_DN;
        }
      } else {
        // other side, check relation.
        switch(G.side) {
          case S_FT: {
            switch(G.tside) {
              case S_RT: G.dir = D_RT; break;
              case S_LT: G.dir = D_LT; break;
              case S_TP: G.dir = D_UP; break;
              case S_BT: G.dir = D_DN; break;
            }
          } break;
          case S_RT: {
            switch(G.tside) {
              case S_FT: G.dir = D_LT; break;
              case S_BK: G.dir = D_RT; break;
              case S_TP: G.dir = D_UP; break;
              case S_BT: G.dir = D_DN; break;
            }
          } break;
          case S_BK: {
            switch(G.tside) {
              case S_RT: G.dir = D_LT; break;
              case S_LT: G.dir = D_RT; break;
              case S_TP: G.dir = D_UP; break;
              case S_BT: G.dir = D_DN; break;
            }
          } break;
          case S_LT: {
            switch(G.tside) {
              case S_FT: G.dir = D_RT; break;
              case S_BK: G.dir = D_LT; break;
              case S_TP: G.dir = D_UP; break;
              case S_BT: G.dir = D_DN; break;
            }
          } break;
          case S_TP: {
            switch(G.tside) {
              case S_FT: G.dir = D_DN; break;
              case S_RT: G.dir = D_RT; break;
              case S_BK: G.dir = D_UP; break;
              case S_LT: G.dir = D_LT; break;
            }
          } break;
          case S_BT: {
            switch(G.tside) {
              case S_FT: G.dir = D_UP; break;
              case S_RT: G.dir = D_RT; break;
              case S_BK: G.dir = D_DN; break;
              case S_LT: G.dir = D_LT; break;
            }
          } break;
        }
      }












    }
  }
});

var nextgamestep = 0;


function tick() {
  var T = (new Date()).getTime() / 1000.0;

  if (G.starttime == 0)
    G.starttime = T;

  if (G.lasttick == 0)
    G.lasttick = T;

  var DT = T - G.lasttick;

  G.lasttick = T;

  if (T > nextgamestep) {
    if (nextgamestep != 0) stepGame();
    nextgamestep = T + 0.3;
  }

  // build pixelgrid

  var pixels = [];
  for(var i=0; i<6*16; i++) {
    pixels.push(0);
  }

  pixels[G.side * 16 + G.y * 4 + G.x] = 1;

  if ((G.time % 1.0) < 0.5) {
    pixels[G.aside * 16 + G.ay * 4 + G.ax] = 1;
  }

  if ((G.time % 1.0) > 0.7) {
    pixels[G.tside * 16 + G.ty * 4 + G.tx] = 1;
  }

  G.history.forEach(function(h) {
    pixels[h.side * 16 + h.y * 4 + h.x] = 1;
  });

  cube.setLeds(pixels);

  G.time += DT;
}


setInterval(function() {
  tick();
}, 100);



//
// Game loop
//


//
// Application is running...
//
