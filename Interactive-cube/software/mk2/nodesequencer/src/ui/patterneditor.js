function PatternEditorUI(ids) {
	this.ids = ids;
	this.display = null;
	this.song = null;
	this.bank = 0;
}

PatternEditorUI.prototype.isStepOn = function(index) {
	var track = this.song.tracks[0];
	var pattern = track.patterns[this.song.editPattern];
	var step = pattern.steps[index];
	var selectnote = this.song.editInstrument + 36;

	var found = -1;
	for(var i=0; i<step.notes.length; i++) {
		var note = step.notes[i];
		if (note.note == selectnote) {
			found = i;
		}
	}

	var isstepcurrent = (index == this.song.currentStep)

	return (found != -1) ^ isstepcurrent;
}

PatternEditorUI.prototype.toggleStep = function(index) {
	var track = this.song.tracks[0];
	var pattern = track.patterns[this.song.editPattern];
	var step = pattern.steps[index];
	var selectnote = this.song.editInstrument + 36;

	var found = -1;
	for(var i=0; i<step.notes.length; i++) {
		var note = step.notes[i];
		if (note.note == selectnote) {
			found = i;
		}
	}

	if (found == -1) {
		// add note
		step.notes.push({
			note: selectnote,
		})
	} else {
		// remove note
		step.notes.splice(found, 1);
	}

	return found;
}

PatternEditorUI.prototype.buttonDown = function(index) {
	console.log('PatternEditorUI::buttonDown', index);
	for(var i=0; i<16; i++) {
		if (index == this.ids[i]) this.toggleStep(i);
	}
}

PatternEditorUI.prototype.buttonUp = function(index) {
}

PatternEditorUI.prototype.render = function() {
	// console.log('PatternEditorUI::render');
	for(var i=0; i<16; i++) {
		this.display.setLed(this.ids[i], this.isStepOn(i));
	}
}

exports.PatternEditorUI = PatternEditorUI;

