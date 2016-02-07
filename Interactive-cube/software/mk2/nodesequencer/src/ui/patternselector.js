function PatternSelectorUI(ids) {
	this.ids = ids;
	this.display = null;
	this.song = null;
	this.tempo = 0;
}

PatternSelectorUI.prototype.buttonDown = function(index) {
	console.log('PatternSelectorUI::buttonDown', index);
	if (index == this.ids[0]) { this.song.editPattern = 0; }
	if (index == this.ids[1]) { this.song.editPattern = 1; }
	if (index == this.ids[2]) { this.song.editPattern = 2; }
	if (index == this.ids[3]) { this.song.editPattern = 3; }
}

PatternSelectorUI.prototype.buttonUp = function(index) {
}

PatternSelectorUI.prototype.render = function() {
	// console.log('PatternSelectorUI::render');
	this.display.setLed(this.ids[0], (this.song.editPattern == 0));
	this.display.setLed(this.ids[1], (this.song.editPattern == 1));
	this.display.setLed(this.ids[2], (this.song.editPattern == 2));
	this.display.setLed(this.ids[3], (this.song.editPattern == 3));
}

exports.PatternSelectorUI = PatternSelectorUI;

