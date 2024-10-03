//UCHUCK() - this will expose this ChucK file as an asset to Chunreal - Work in progress
// feedforward
adc.left => Gain gL => dac.right;
adc.right => Gain gR => dac.left;
// feedback
gL => Gain feedbackL => DelayL delayL => gL;
gR => Gain feedbackR => Delay delayR => gR;

0.1 => global float DelayTime;

global Event paramUpdate;
Event Test;

0.8 => float FeedbackGain ;
// set feedback
FeedbackGain => feedbackL.gain;
FeedbackGain => feedbackR.gain;
// set effects mix

1 => float mixGain ;

mixGain => delayL.gain;
mixGain => delayR.gain;

// infinite time loop
<<< "Initializing Chuck" >>>;
fun void SetParams ( Event ee)
{
	ee => now;
	DelayTime::second => delayL.max => delayL.delay;
	DelayTime::second => delayR.max => delayR.delay;
	<<< "Params updated from event" , DelayTime >>>;
	

}

spork ~ SetParams(paramUpdate);
paramUpdate.broadcast();


while( true ) 
	{
	1::samp => now;
	paramUpdate.broadcast();
	}
