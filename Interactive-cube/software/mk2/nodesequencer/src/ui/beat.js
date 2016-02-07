function BeatUI(ids) {
	this.ids = ids;
	this.display = null;
	this.song = null;
	this.bank = 0;
}

BeatUI.prototype.buttonDown = function(index) {
}

BeatUI.prototype.buttonUp = function(index) {
}

BeatUI.prototype.render = function() {
	// console.log('BeatUI::render');
	for(var i=0; i<16; i++) {
		this.display.setLed(this.ids[i], (i % 4) < (4 - (this.song.currentStep % 4)));
	}
}

exports.BeatUI = BeatUI;

