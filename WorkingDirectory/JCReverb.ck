// stereo reverb
adc => JCRev revL => LPF Lpf => Gain gL => dac;
//adc => JCRev revR => Gain gR => dac.right;


// set effects mix
1500 => Lpf.freq;
1  => float mixGain;

mixGain => revL.gain;
//mixGain => revR.gain;

// infinite time loop
while( true ) 1::second => now;