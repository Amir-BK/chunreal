// Chunreal Midi Drum Synth
// Author: Amir Ben-Kiki 
// This will will be a bit of a template for ChucK instruments in unreal 

global Event noteEvent;
global float noteFreq;

//

// set up patch and samples, outside the main loop, happens once:
Gain g => dac; // create a mixer and send it to the output buffer 
Gain g2 => dac;

SndBuf kick => g => dac;
SndBuf snare=> g =>   dac;
SndBuf hihat => g => dac;

snare => Delay delay(0.3::second, 0.1::second) => g2 => dac;
1 => g2.gain;
// read files, we use me.dir() + "path_in_side_working_directory" syntax to get relative file paths that we can package with the plugin or a game

me.dir() + "drumkit/kick-0.wav" => kick.read;
me.dir() + "drumkit/snare-0.wav" => snare.read;
me.dir() + "drumkit/hihat-0.wav" => hihat.read;

//we set the play position to the end of the file so that they don't initially play, playback will be achieved by set the position back to zero
kick.samples() => kick.pos;
snare.samples() => snare.pos;
hihat.samples() => hihat.pos;


// infinite time-loop
while( true )
{
    //whenever the noteEvent is called we 
    noteEvent => now;
    play( noteFreq, Std.rand2f( .6, .9 ) );


}
// test 
// basic play function (add more arguments as needed)
fun void play( float note, float velocity )
{
   // <<< "chuck drum note" , note >>>;
    // soon we'll check the velocity but for now just set the pos of the kick to 0
    if(note == 36)
    {
           00=>kick.pos;
    }

    if(note == 38)
    {
        00=>snare.pos;
    }
 
}