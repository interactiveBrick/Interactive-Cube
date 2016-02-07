function LiveTriggerUI(ids) {
	this.display = null;
	this.player = null;
	this.ids = ids;
}

LiveTriggerUI.prototype.buttonDown = function(index) {
	for(var i=0; i<16; i++) {
		if (index == this.ids[i]) this.player.triggerNote(i + 36, this.song.editChannel);
	}
}

LiveTriggerUI.prototype.buttonUp = function(index) {
}

LiveTriggerUI.prototype.render = function() {
	for(var i=0; i<16; i++) {
		this.display.setLed(this.ids[i], this.player.isNotePlaying(i + 36, this.song.editChannel));
	}
}

exports.LiveTriggerUI = LiveTriggerUI;

