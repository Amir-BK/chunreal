// stereo reverb
//adc =>  NRev  revL => dac;

adc.left => Gain gL =>  GVerb     revL  =>  dac.left;
adc.right => Gain gR =>  GVerb     revR => dac.right;

1 => revL.gain;
//adc.right =>  NRev  revR => dac.right;
//Math.
// set effects mix//
//
//fun void UCHUCK() {};

while(true) 1::second => now;

