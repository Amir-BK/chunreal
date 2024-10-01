//UCHUCK() - this will expose this ChucK file as an asset to Chunreal - Work in progress
// feedforward
adc.left => Gain gL => dac.left;
adc.right => Gain gR => dac.right;
// feedback
gL => Gain feedbackL => DelayL delayL => gL;
gR => Gain feedbackR => DelayL delayR => gR;

0.5 => global float delaytimeleft;

global Event paramUpdate;
Event Test;

// set feedback
0.8 => feedbackL.gain;
0.8 => feedbackR.gain;
// set effects mix

1 => float mixGain ;

mixGain => delayL.gain;
mixGain => delayR.gain;
<<< delaytimeleft>>>;
// infinite time loop

fun void SetParams ( Event ee)
{
	ee => now;
	delaytimeleft::second => delayL.max => delayL.delay;
	delaytimeleft::second => delayR.max => delayR.delay;
	<<< "Params updated from event" , delaytimeleft >>>;
	

}

spork ~ SetParams(paramUpdate);

while( true ) 1::second => now;
