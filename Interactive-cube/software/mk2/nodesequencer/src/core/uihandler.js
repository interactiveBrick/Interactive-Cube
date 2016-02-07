function UIHandler() {
	this.display = null;
	this.song = null;
	this.uis = [];
	this.counter = 0;
}

UIHandler.prototype.buttonDown = function(index) {
	var _this = this;
	this.uis.forEach(function(x) {
		x.player = _this.player;
		x.buttonDown(index);
	});
}

UIHandler.prototype.buttonUp = function(index) {
	var _this = this;
	this.uis.forEach(function(x) {
		x.player = _this.player;
		x.buttonUp(index);
	});
}

UIHandler.prototype.render = function() {
	var _this = this;
	this.uis.forEach(function(x) {
		x.player = _this.player;
		x.render();
	});
	this.counter ++;
	this.display.render((this.counter % 100) == 0);
}

UIHandler.prototype.addUI = function(ui) {
	this.uis.push(ui);
	ui.display = this.display;
	ui.player = this.player;
	ui.song = this.song;
}

UIHandler.prototype.start = function() {
	this.queuetick();
}

UIHandler.prototype.queuetick = function() {
	setTimeout(this.tick.bind(this), 25);
}

UIHandler.prototype.tick = function() {
	this.render();
	this.queuetick();
}

exports.UIHandler = UIHandler;

