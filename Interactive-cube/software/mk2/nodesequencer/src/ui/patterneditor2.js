function PatternEditorUI2(ids, startcol, startrow) {
	this.ids = ids;
	this.display = null;
	this.song = null;
	this.bank = 0;
	this.startrow = startrow;
	this.startcol = startcol;
}

PatternEditorUI2.prototype.isStepOn = function(col, row) {
	var track = this.song.tracks[0];
	var pattern = track.patterns[this.song.editPattern];
	var step = pattern.steps[this.startcol + col];
	var selectnote = this.song.editInstrument + ((row + this.startrow) % 16) + 36;

	var found = -1;
	for(var i=0; i<step.notes.length; i++) {
		var note = step.notes[i];
		if (note.note == selectnote && note.channel == this.song.editChannel) {
			found = i;
		}
	}

	var isstepcurrent = ((this.startcol + col) == this.song.currentStep)

	return (found != -1) ^ isstepcurrent;
}

PatternEditorUI2.prototype.toggleStep = function(col, row) {
	var track = this.song.tracks[0];
	var pattern = track.patterns[this.song.editPattern];
	var step = pattern.steps[this.startcol + col];
	var selectnote = this.song.editInstrument + ((row + this.startrow) % 16) + 36;

	var found = -1;
	for(var i=0; i<step.notes.length; i++) {
		var note = step.notes[i];
		if (note.note == selectnote && note.channel == this.song.editChannel) {
			found = i;
		}
	}

	if (found == -1) {
		// add note
		step.notes.push({
			note: selectnote,
			channel: this.song.editChannel,
		})
	} else {
		// remove note
		step.notes.splice(found, 1);
	}

	return found;
}

PatternEditorUI2.prototype.buttonDown = function(index) {
	console.log('PatternEditorUI2::buttonDown', index);
	for(var i=0; i<16; i++) {
		if (index == this.ids[i]) this.toggleStep(i % 4, i >> 2);
	}
}

PatternEditorUI2.prototype.buttonUp = function(index) {
}

PatternEditorUI2.prototype.render = function() {
	// console.log('PatternEditorUI2::render');
	for(var i=0; i<16; i++) {
		this.display.setLed(this.ids[i], this.isStepOn(i % 4, i >> 2));
	}
}

exports.PatternEditorUI2 = PatternEditorUI2;

