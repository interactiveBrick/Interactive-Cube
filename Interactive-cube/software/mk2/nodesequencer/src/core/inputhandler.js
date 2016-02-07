function InputHandler() {
	this.buttonstate = new Array(4*4*6);
	for(var i=0; i<4*4*6; i++) {
		this.buttonstate[i] = false;
	}
	this.listeners = [];
}

InputHandler.prototype.addListener = function(handler) {
	this.listeners.push(handler);
}

InputHandler.prototype.buttonDown = function(index) {
	this.listeners.forEach(function(x) {
		x.buttonDown(index);
	});
}

InputHandler.prototype.buttonUp = function(index) {
	this.listeners.forEach(function(x) {
		x.buttonUp(index);
	});
}

InputHandler.prototype.start = function() {
}

exports.InputHandler = InputHandler;
