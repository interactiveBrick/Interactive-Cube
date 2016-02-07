function PatternCueSelectorUI(ids) {
	this.ids = ids;
	this.display = null;
	this.song = null;
	this.tempo = 0;
}

PatternCueSelectorUI.prototype.buttonDown = function(index) {
	console.log('PatternCueSelectorUI::buttonDown', index);
	if (index == this.ids[0]) { this.song.cuedPattern = 0; }
	if (index == this.ids[1]) { this.song.cuedPattern = 1; }
	if (index == this.ids[2]) { this.song.cuedPattern = 2; }
	if (index == this.ids[3]) { this.song.cuedPattern = 3; }
}

PatternCueSelectorUI.prototype.buttonUp = function(index) {
}

PatternCueSelectorUI.prototype.render = function() {
	// console.log('PatternCueSelectorUI::render');

	var beat = (this.song.currentStep % 4) < 2;
	this.display.setLed(this.ids[0], (this.song.cuedPattern == 0) || (beat && this.song.playingPattern == 0));
	this.display.setLed(this.ids[1], (this.song.cuedPattern == 1) || (beat && this.song.playingPattern == 1));
	this.display.setLed(this.ids[2], (this.song.cuedPattern == 2) || (beat && this.song.playingPattern == 2));
	this.display.setLed(this.ids[3], (this.song.cuedPattern == 3) || (beat && this.song.playingPattern == 3));
}

exports.PatternCueSelectorUI = PatternCueSelectorUI;

