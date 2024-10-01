// feedforward
adc.left => Gain gL => dac.left;
adc.right => Gain gR => dac.right;
// feedback
gL => Gain feedbackL => DelayL delayL => gL;
gR => Gain feedbackR => DelayL delayR => gR;

 0.25  => global float delaytimeleft;

global Event paramUpdate;

// set delay parameters
delaytimeleft::second => delayL.max => delayL.delay;
delaytimeleft::second => delayR.max => delayR.delay;
// set feedback
0.8 => feedbackL.gain;
0.8 => feedbackR.gain;
// set effects mix

1 => float mixGain;

mixGain => delayL.gain;
1 => delayR.gain;

// infinite time loop
while( true ) { 	
	paramUpdate=> now;
	delaytimeleft::second => delayL.max => delayL.delay;
	delaytimeleft::second => delayR.max => delayR.delay;
	<<< "Params updated from event" >>>;
	1::second => now;
//delaytimeleft::second => delayL.max => delayL.delay;
//delaytimeleft::second => delayR.max => delayR.delay;
//<<< delaytimeleft>>>;
}
//////////////////////