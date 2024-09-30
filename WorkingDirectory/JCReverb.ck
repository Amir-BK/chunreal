// should be very simple
JCRev jcRev;

adc  => jcRev =>  dac;
//adc  => blackhole;
//adc   =>  dac;
<<< "Test String adg" >>>;
// set wet/dry ratio
.1 => float revMix;
revMix => jcRev.mix;

<<< dac >>>;

				