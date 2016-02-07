
function DisplayHandler() {
	this.player = null;
	this.displaydirty = true;
	this.lastleds = new Array(4*4*6);
	this.leds = new Array(4*4*6);
	for(var i=0; i<4*4*6; i++) {
		this.leds[i] = false;
		this.lastleds[i] = false;
	}
	this.displays = [];
	this.uis = [];
	this.counter = 0;
}

DisplayHandler.prototype.addDisplay = function(handler) {
	this.displays.push(handler);
}

DisplayHandler.prototype.addUI = function(ui) {
	this.uis.push(ui);
}

DisplayHandler.prototype.setLed = function(index, on) {
	if (on == this.leds[index]) {
		return;
	}

	this.leds[index] = on;
	this.displaydirty = true;
}

DisplayHandler.prototype.render = function(force) {
	if (!this.displaydirty && !force) {
		return;
	}

	this.displaydirty = false;

	var changes = [];
	for(var i=0; i<4*4*6; i++) {
		if (this.leds[i] != this.lastleds[i]) {
			changes.push({ led: i, on: this.leds[i] });
		}
		this.lastleds[i] = this.leds[i];
	}

	// add some extra updates too
	for(var k=0; k<3; k++) {
		changes.push({ led: this.counter, on: this.leds[this.counter] });

		this.counter ++;
		if (this.counter > 4*4*6)
			this.counter = 0;
	}

	// console.log('DisplayHandler: render', this.leds.map(function(x) { return x ? 1 : 0; }).join(','));
	// console.log('changes', changes);

	var _this = this;
	this.displays.forEach(function(x) {
		x.player = _this.player;
		x.render({ leds: _this.leds, changes: changes });
	});
}

exports.DisplayHandler = DisplayHandler;

