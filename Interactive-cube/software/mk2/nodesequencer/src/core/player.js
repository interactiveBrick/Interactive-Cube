// var microtime = require('microtime');

function Player() {
	this.song = null;
	this.bpm = 120;
	this.secondsperstep = 0;
	this.step = -1;
	this.substep = 0;
	this.destinations = [];
	this.lasttick = 0;
	this.activenotes = [];
	this.lastnotetime = {};
}

Player.prototype.start = function() {
	this.queuetick();
}

Player.prototype.addDestination = function(handler) {
	this.destinations.push(handler);
}

Player.prototype.queuetick = function() {
	setTimeout(this.tick.bind(this), 10);
}

Player.prototype.send = function(events) {
	// console.log('send', events);
	this.destinations.forEach(function(x) {
		x.send(events);
	});
}

Player.prototype.isNotePlaying = function(note, channel) {
	var T = (new Date()).getTime() / 1000.0;
	var lasttrig = (this.lastnotetime[channel * 256 + note] || 0);
	var delta = (T - lasttrig);
	return delta < 0.4;
}

Player.prototype.triggerNote = function(note, channel) {
	this.send([{
		on: true,
		note: note,
		channel: channel,
	}]);

	this.activenotes.push({
		note: note,
		expires: 0.2,
		channel: channel
	});

	var T = (new Date()).getTime() / 1000.0;
	this.lastnotetime[channel * 256 + note] = T;
}

Player.prototype.advanceStep = function() {
	this.step ++;
	if (this.step >= 16) {
		this.step = 0; // switch pattern if anything queued
		if (this.song.cuedPattern != -1) {
			this.song.playingPattern = this.song.cuedPattern;
			this.song.cuedPattern = -1;
		}
	}

	this.song.currentStep = this.step;// (this.step + 16) % 16;

	// console.log('advanceStep', this.step>>4, this.step%4, this.step);

	for(var k=0; k<4; k++) {
		var track = this.song.tracks[k];
		var pattern = track.patterns[this.song.playingPattern];
		var step = pattern.steps[(this.step + 16) % 16];
		if (step.notes.length > 0) {
			// console.log('stepinfo', step);
			for(var i=0; i<step.notes.length; i++) {
				this.triggerNote(step.notes[i].note, step.notes[i].channel);
			}
		}
	}
}

Player.prototype.tick = function() {
	this.secondsperstep = 60 / (this.bpm * 4);
	var T = (new Date()).getTime() / 1000.0;
	var dT = 0;
	if (this.lasttick != 0) {
		dT = T - this.lasttick;
	}
	this.substep += dT;
	if (this.substep > this.secondsperstep) {
		this.advanceStep();
		this.substep -= this.secondsperstep;
	}
	this.lasttick = T;

	for(var j=this.activenotes.length-1; j>=0; j--) {
		this.activenotes[j].expires -= dT;
		if (this.activenotes[j].expires < 0.0) {
			// note off
			// console.log('Send note off:', this.notes[j].note);
			// this.output.sendMessage([ 0x80, this.notes[j].note, 0 ]);
			this.send([{
				on: false,
				note: this.activenotes[j].note,
				channel: this.activenotes[j].channel,
			}]);

			// remove.
			this.activenotes.splice(j, 1);
		}
	}


	this.queuetick();
	this.destinations.forEach(function(x) {
		x.tick(dT);
	});
}

exports.Player = Player;
