//Create a global event and an oscillator
global Event customEvent;
global float noteFreq;
//SinOsc osc => dac;

100 => global float freq;
//freq => osc.freq;

// patch
//Mandolin m => JCRev r => dac;
//.75 => r.gain;
//.05 => r.mix;


Rhodey voc => JCRev r => Echo a => Echo b => Echo c => dac;

220.0 => voc.freq;
0.8 => voc.gain;
.8 => r.gain;
.2 => r.mix;
1000::ms => a.max => b.max => c.max;
750::ms => a.delay => b.delay => c.delay;
.1 => a.mix => b.mix => c.mix;

// infinite time-loop
while( true )
{
    
    customEvent => now;
    

    // factor
    Std.rand2f( 1, 4 ) => float factor;


    play( noteFreq, Std.rand2f( .6, .9 ) );
    //100::ms * factor => now;

}

// basic play function (add more arguments as needed)
fun void play( float note, float velocity )
{
    // start the note
    Std.mtof( note ) => voc.freq;
    velocity => voc.noteOn;
}