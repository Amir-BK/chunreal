// stereo reverb
adc => JCRev revL =>  Gain gL => dac.left;
adc => JCRev revR => Gain gR => dac.right;


// set effects mix

1  => float mixGain;

mixGain => revL.gain;
mixGain => revR.gain;

// infinite time loop
while( true ) 1::second => now;