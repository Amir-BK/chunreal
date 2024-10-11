// stereo reverb
adc.left =>  LPF LpfL => Gain gL => JCRev revL => dac.left;
adc.right =>  LPF LpfR => Gain gR => JCRev revR => dac.right;
//Math.
// set effects mix
//
//fun void UCHUCK() {};

//Low pass frequency 
2000 => float f;
//keep at 1 if using as parallel reverb and control reverb amount via mix, if used as insert this is the wet/dry control.
0.1  => float mixGain;

//JCRev output needs to be low otherwise it causes saturation or something? idgi
0.1 => float dacGain;

dacGain => gL.gain;
dacGain => gR.gain;

f => LpfL.gain;
f => LpfR.freq;
//evL.

mixGain => revL.gain;
mixGain => revR.gain;

// infinite time loop
while( true ) 1::second => now;


