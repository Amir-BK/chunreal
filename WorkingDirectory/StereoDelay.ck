// feedforward
adc.left => Gain gL => dac.left;
adc.right => Gain gR => dac.right;
// feedback
gL => Gain feedbackL => DelayL delayL => gL;
gR => Gain feedbackR => DelayL delayR => gR;

// set delay parameters
0.3::second => delayL.max => delayL.delay;
0.5::second => delayR.max => delayR.delay;
// set feedback
0.8 => feedbackL.gain;
0.8 => feedbackR.gain;
// set effects mix

1 => float mixGain;

mixGain => delayL.gain;
1 => delayR.gain;

// infinite time loop
while( true ) 1::second => now;

//////////////////