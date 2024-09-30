// feedforward
adc => Gain gL => dac.left;
adc => Gain gR => dac.right;
// feedback
gL => Gain feedbackL => DelayL delayL => gL;
gR => Gain feedbackR => DelayL delayR => gR;

// set delay parameters
.75::second => delayL.max => delayL.delay;
.95::second => delayR.max => delayR.delay;
// set feedback
.5 => feedbackL.gain;
.5 => feedbackR.gain;
// set effects mix

0.9 => float mixGain;

mixGain => delayL.gain;
mixGain => delayR.gain;

// infinite time loop
while( true ) 1::second => now;