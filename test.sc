// Set control rate to DDSP rate
(
	o = Server.default.options;
	o.blockSize = 192;
	Server.default.reboot;
)

// Setup stereo Convolutional Reverb
(
~fftsize = 2048;
{
	var l_irbuffer, r_irbuffer, bufsize;

	l_irbuffer = Buffer.readChannel(s,
		"/home/kureta/Documents/repos/performer/notebooks/cello_controller-ir.wav",
		channels: [0]
	);
	s.sync;

	bufsize = PartConv.calcBufSize(~fftsize, l_irbuffer);

	~l_irspectrum = Buffer.alloc(s, bufsize, 1);
	~l_irspectrum.preparePartConv(l_irbuffer, ~fftsize);
	s.sync;

	l_irbuffer.free;

	r_irbuffer = Buffer.readChannel(s,
		"/home/kureta/Documents/repos/performer/notebooks/cello_controller-ir.wav",
		channels: [1]
	);
	s.sync;

	~r_irspectrum = Buffer.alloc(s, bufsize, 1);
	~r_irspectrum.preparePartConv(r_irbuffer, ~fftsize);
	s.sync;

	r_irbuffer.free;
}.fork;
)

// Setup Controller and DSP elements
({
	var input = SinOsc.ar(
		SinOsc.kr(4.0, 0.0, 10.0, 440.0),
		0.0,
		Performer.kr(
			SinOsc.kr(1.0, 0.0, 10.0, 440.0),
			Phaser.kr(0.333, 0.0, 70.0, -80.0)
		)
	);
	var left = PartConv.ar(input, ~fftsize, ~l_irspectrum.bufnum);
	var right = PartConv.ar(input, ~fftsize, ~r_irspectrum.bufnum);
	[left, right];
}.play
)