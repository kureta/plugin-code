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
		"/home/kureta/Documents/repos/performer/out/cello-ir.wav",
		channels: [0]
	);
	s.sync;

	bufsize = PartConv.calcBufSize(~fftsize, l_irbuffer);

	~l_irspectrum = Buffer.alloc(s, bufsize, 1);
	~l_irspectrum.preparePartConv(l_irbuffer, ~fftsize);
	s.sync;

	l_irbuffer.free;

	r_irbuffer = Buffer.readChannel(s,
		"/home/kureta/Documents/repos/performer/out/cello-ir.wav",
		channels: [1]
	);
	s.sync;

	~r_irspectrum = Buffer.alloc(s, bufsize, 1);
	~r_irspectrum.preparePartConv(r_irbuffer, ~fftsize);
	s.sync;

	r_irbuffer.free;
}.fork;
)

({Loudness.kr(
	FFT(LocalBuf(1024), SoundIn.ar(0))
)}.play;)

// Setup Controller and DSP elements
// SinOsc.kr(0.333, 0.0, 25.0, -60.0)
({
	var mic, input, left, right, freq, hasFreq;
	mic = SoundIn.ar(0);
	# freq, hasFreq = Pitch.kr(mic, ampThreshold: 0.01, median: 7);
	input = Performer.ar(
		freq,
		Loudness.kr(FFT(LocalBuf(1024), mic)).log2 * 10 - 60
	);
	// var input = SinOsc.ar() * 0.01;
	left = PartConv.ar(input, ~fftsize, ~l_irspectrum.bufnum);
	right = PartConv.ar(input, ~fftsize, ~r_irspectrum.bufnum);
	[left, right];
}.play
)
