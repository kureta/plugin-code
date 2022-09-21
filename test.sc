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
		"/home/kureta/Documents/repos/performer/out/violin-ir.wav",
		channels: [0]
	);
	s.sync;

	bufsize = PartConv.calcBufSize(~fftsize, l_irbuffer);

	~l_irspectrum = Buffer.alloc(s, bufsize, 1);
	~l_irspectrum.preparePartConv(l_irbuffer, ~fftsize);
	s.sync;

	l_irbuffer.free;

	r_irbuffer = Buffer.readChannel(s,
		"/home/kureta/Documents/repos/performer/out/violin-ir.wav",
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

// Setup Controller and DSP elements
(
x = {
	arg t_gate=0, freq=440.0;
	var env, dry, left, right, hasFreq;
	env = EnvGen.kr(Env.perc(0.01, 2.0).range(-110, -36) , t_gate);
	dry = Performer.ar(
		freq + SinOsc.kr(5, 0, freq * 1.014545335 - freq),
		env
	);
	// var dry = SinOsc.ar() * 0.01;
	left = PartConv.ar(dry, ~fftsize, ~l_irspectrum.bufnum);
	right = PartConv.ar(dry, ~fftsize, ~r_irspectrum.bufnum);
	[left, right];
}.play;
)

(
p = Pbind(
    \type, \set,    // This tells it we'll be setting parameters of an existing node...
    \id, x.nodeID,    // ...this tells it whose parameters we'll be setting
    \args, #[\t_gate, \freq],  // and this tells it which parameters to set
	\freq, Pwhite(440, 1670),
    \dur, 0.125,
	\t_gate, Pwhite(1, 1)
).play;
)
(
p.stop;
x.free;
)



