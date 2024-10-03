// stereo reverb
adc => JCRev revL => LPF LpfL => Gain gL => dac.left;
adc => JCRev revR => LPF LpfR => Gain gR => dac.right;


// set effects mix

//Low pass frequency 
20000 => float f;
//keep at 1 if using as parallel reverb and control reverb amount via mix, if used as insert this is the wet/dry control.
0.999  => float mixGain;

//JCRev output needs to be low otherwise it causes saturation or something? idgi
0.3 => float dacGain;

dacGain => gL.gain;
dacGain => gR.gain;

f => LpfL.freq;
f => LpfR.freq;


mixGain => revL.gain;
mixGain => revR.gain;

// infinite time loop
while( true ) 1::second => now;