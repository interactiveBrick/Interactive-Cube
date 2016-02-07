
function CubeUI() {
	this.displaydirty = false;
	this.leds = new Array(4*4*6);
	this.buttonstate = new Array(4*4*6);
	for(var i=0; i<4*4*6; i++) {
		this.leds[i] = false;
		this.buttonstate[i] = false;
	}
}

CubeUI.prototype.buttonDown = function(index) {
}

CubeUI.prototype.buttonUp = function(index) {
}

CubeUI.prototype.setLed = function(index, on) {
	if (on != this.leds[index]) {
		this.leds[index] = on;
		this.displaydirty = true;
	}
}

CubeUI.prototype.getLed = function(index) {
	return this.leds[index];
}

export.CubeUI = CubeUI;