//
// Node.js based sequencer example
//

var midi = require('midi');

var CubeManager = require('../nodelib/cube/index.js').CubeManager;

var Song = require('./src/core/song').Song;
var Player = require('./src/core/player').Player;

var DisplayHandler = require('./src/core/displayhandler.js').DisplayHandler;
var InputHandler = require('./src/core/inputhandler.js').InputHandler;
var UIHandler = require('./src/core/uihandler.js').UIHandler;

var PatternSelectorUI = require('./src/ui/patternselector.js').PatternSelectorUI;
var InstrumentSelectorUI = require('./src/ui/instrumentselector.js').InstrumentSelectorUI;
var PatternEditorUI = require('./src/ui/patterneditor.js').PatternEditorUI;
var PatternEditorUI2 = require('./src/ui/patterneditor2.js').PatternEditorUI2;
var PatternCueSelectorUI = require('./src/ui/patterncueselector.js').PatternCueSelectorUI;

var TrackEditorUI = require('./src/ui/trackeditor.js').TrackEditorUI;
var LiveTriggerUI = require('./src/ui/livetrigger.js').LiveTriggerUI;
var LevelEditorUI = require('./src/ui/leveleditor.js').LevelEditorUI;

var BeatUI = require('./src/ui/beat.js').BeatUI;



//
// Set up application
//

var song = new Song();
var player = new Player();
var display = new DisplayHandler();
var input = new InputHandler();
var ui = new UIHandler();



//
// Set up cube connection
//

var cubemanager = new CubeManager();
cubemanager.listen(3333);

var cube = cubemanager.getCube('cube18FE');
cube.connect('192.168.0.88', 3333);

cube.addListener(function(msg) {
  if (msg[0] == '/' + cube.id + '/btn') {
    if (msg[2] == 1) {
      input.buttonDown(msg[1]);
    } else {
      input.buttonUp(msg[1]);
    }
  }
});

cube.startDebugWebServer(3000);




//
// Set up MIDI output
//

function MIDIDestination() {
  // this.notes = [];

  this.output = new midi.output();
  console.log('MIDI Destination: ' + this.output.getPortName(0));
  this.output.openPort(0);
}

MIDIDestination.prototype.tick = function(dT) {
}

MIDIDestination.prototype.send = function(events) {
  var _this = this;
  events.forEach(function(event) {
    _this.output.sendMessage([ (event.on ? 0x90 : 0x80) + event.channel, event.note, 100 ]);
  });
}

player.addDestination(new MIDIDestination());



//
// Set up OSC display
//

function CubeDisplayAdapter() {}

CubeDisplayAdapter.prototype.render = function(displayinfo) {
  cube.setLeds(displayinfo.leds);
}

display.addDisplay(new CubeDisplayAdapter());



//
// Set up input controls
//

player.song = song;
player.bpm = 125;

display.player = player;

ui.player = player;
ui.display = display;
ui.song = song;

// ui.addUI(new PatternSelectorUI([ 12,13,14,15 ]));
// ui.addUI(new InstrumentSelectorUI([0,1,2,3, 4,5,6,7]));
// ui.addUI(new PatternEditorUI([ 16,17,18,19, 20,21,22,23, 24,25,26,27, 28,29,30,31 ]));

function IDGen(side, startcol, startrow, numcols, numrows) {
  var ret = [];
  for(var j=0; j<numrows; j++) {
    for(var i=0; i<numcols; i++) {
      var y = startrow + j;
      var x = startcol + i;
      ret.push((side * 16) + (y * 4) + x);
    }
  }
  return ret;
}

ui.addUI(new PatternSelectorUI(IDGen( 4, 0,3, 4,1 )));
ui.addUI(new InstrumentSelectorUI(IDGen( 4, 0,0, 4,2 )));
ui.addUI(new PatternCueSelectorUI(IDGen( 4, 0,2, 4,1 )));

ui.addUI(new PatternEditorUI2(IDGen( 0, 0,0, 4,4 ), 0, 0));
ui.addUI(new PatternEditorUI2(IDGen( 1, 0,0, 4,4 ), 4, 0));
ui.addUI(new PatternEditorUI2(IDGen( 2, 0,0, 4,4 ), 8, 0));
ui.addUI(new PatternEditorUI2(IDGen( 3, 0,0, 4,4 ), 12, 0));

ui.addUI(new LiveTriggerUI(IDGen( 5, 0,0, 4,4 )));

// ui.addUI(new BeatUI([ 64,65,66,67, 68,69,70,71, 72,73,74,75, 76,77,78,79 ]));
// ui.addUI(new BeatUI([ 80,81,82,83, 84,85,86,87, 88,89,90,91, 92,93,94,95 ]));
// ui.addUI(new PatternEditorUI([ 0,1,2,3, 4,5,6,7, 8,9,10,11, 12,13,14,15 ]));

input.addListener(ui);



//
// Start everything
//

input.start();
player.start();
ui.start();


//
// Mock track
//

song.tracks[0].patterns[0].steps[0].notes = [ { note: 36, channel: 0 } ];
song.tracks[0].patterns[0].steps[4].notes = [ { note: 36, channel: 0 }, { note: 38, channel: 0 } ];
song.tracks[0].patterns[0].steps[8].notes = [ { note: 36, channel: 0 } ];
song.tracks[0].patterns[0].steps[12].notes = [ { note: 36, channel: 0 }, { note: 38, channel: 0 } ];

/*
song.tracks[0].patterns[0].steps[2].notes = [ { note: 44, channel: 0 } ];
song.tracks[0].patterns[0].steps[6].notes = [ { note: 44, channel: 0 } ];
song.tracks[0].patterns[0].steps[10].notes = [ { note: 44, channel: 0 } ];
song.tracks[0].patterns[0].steps[14].notes = [ { note: 44, channel: 0 } ];

song.tracks[0].patterns[0].steps[13].notes = [ { note: 40, channel: 0 } ];
*/

//
// Application is running...
//
