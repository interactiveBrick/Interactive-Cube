function InstrumentSelectorUI(ids) {
	this.ids = ids;
	this.display = null;
	this.song = null;
	this.bank = 0;
}

InstrumentSelectorUI.prototype.buttonDown = function(index) {
	console.log('InstrumentSelectorUI::buttonDown', index);
	if (index == this.ids[0]) { this.bank = 0; this.song.editInstrument = this.song.editInstrument % 4 + this.bank * 4; }
	if (index == this.ids[1]) { this.bank = 1; this.song.editInstrument = this.song.editInstrument % 4 + this.bank * 4; }
	if (index == this.ids[2]) { this.bank = 2; this.song.editInstrument = this.song.editInstrument % 4 + this.bank * 4; }
	if (index == this.ids[3]) { this.bank = 3; this.song.editInstrument = this.song.editInstrument % 4 + this.bank * 4; }
//	if (index == this.ids[4]) { this.song.editInstrument = (this.bank * 4 + 0); }
//	if (index == this.ids[5]) { this.song.editInstrument = (this.bank * 4 + 1); }
//	if (index == this.ids[6]) { this.song.editInstrument = (this.bank * 4 + 2); }
//	if (index == this.ids[7]) { this.song.editInstrument = (this.bank * 4 + 3); }
	if (index == this.ids[4]) { this.song.editChannel = 0; }
	if (index == this.ids[5]) { this.song.editChannel = 1; }
	if (index == this.ids[6]) { this.song.editChannel = 2; }
	if (index == this.ids[7]) { this.song.editChannel = 3; }
}

InstrumentSelectorUI.prototype.buttonUp = function(index) {
}

InstrumentSelectorUI.prototype.render = function() {
	// console.log('InstrumentSelectorUI::render');
	this.display.setLed(this.ids[0], (this.bank == 0));
	this.display.setLed(this.ids[1], (this.bank == 1));
	this.display.setLed(this.ids[2], (this.bank == 2));
	this.display.setLed(this.ids[3], (this.bank == 3));
//	this.display.setLed(this.ids[4], (this.song.editInstrument == (this.bank * 4 + 0)));
//	this.display.setLed(this.ids[5], (this.song.editInstrument == (this.bank * 4 + 1)));
//	this.display.setLed(this.ids[6], (this.song.editInstrument == (this.bank * 4 + 2)));
//	this.display.setLed(this.ids[7], (this.song.editInstrument == (this.bank * 4 + 3)));
	this.display.setLed(this.ids[4], (this.song.editChannel == 0));
	this.display.setLed(this.ids[5], (this.song.editChannel == 1));
	this.display.setLed(this.ids[6], (this.song.editChannel == 2));
	this.display.setLed(this.ids[7], (this.song.editChannel == 3));
}

exports.InstrumentSelectorUI = InstrumentSelectorUI;

