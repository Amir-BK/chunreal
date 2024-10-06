
SinOsc sin => Gain g => dac;
////////////////////////
global float sinFreq => sin.freq;

while(true) {
	sinFreq => sin.freq;
	0.1::second => now; 
}