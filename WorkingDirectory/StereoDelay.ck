// feedforward
adc => Gain gL => dac.left;
adc => Gain gR => dac.right;
// feedback
gL => Gain feedbackL => DelayL delayL => gL;
gR => Gain feedbackR => DelayL delayR => gR;

// set delay parameters
.175::second => delayL.max => delayL.delay;
0.95::second => delayR.max => delayR.delay;
// set feedback
1.9 => feedbackL.gain;
.9 => feedbackR.gain;
// set effects mix

0.5 => float mixGain;

mixGain => delayL.gain;
mixGain => delayR.gain;

// infinite time loop
while( true ) 1::second => now;

//////////////////