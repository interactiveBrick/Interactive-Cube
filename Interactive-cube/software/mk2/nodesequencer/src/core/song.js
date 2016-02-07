// Song model

function Pattern() {
	this.steps = [];
	for(var i=0; i<16; i++) {
		this.steps.push({
			notes: [],
			/*
			trig: false,
			instrument: 0,
			amplitude: 0,
			note: 36,
			*/
		});
	}
}

function Track() {
	this.patterns = [];
	for(var j=0; j<4; j++) {
		this.patterns.push(new Pattern());
	}
}

function Song() {
	this.playingPattern = 0;
	this.cuedPattern = 0;
	this.currentStep = -1;
	this.editPattern = 0;
	this.editInstrument = 0;
	this.editChannel = 0;
	this.tracks = [];
	for(var j=0; j<4; j++) {
		this.tracks.push(new Track());
	}
}

exports.Song = Song;
