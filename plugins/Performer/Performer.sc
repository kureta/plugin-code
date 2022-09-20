Performer : UGen {
	*ar { |frequency = 440.0, loudnessDb = -30.0|
		^this.multiNew('audio', frequency, loudnessDb);
	}

	checkInputs {
		// Input 0 is frequency
		if(inputs.at(0) == \audio, {
			"You're not supposed to use audio rate here".error
		});
		if(inputs.at(1) == \audio, {
			"You're not supposed to use audio rate here".error
		});

		// Checks if inputs are valid UGen inputs
		// And not a GUI slider or something...
		^this.checkValidInputs;
	}
}
