// Chunreal Midi Drum Synth
// Author: Amir Ben-Kiki 
// This will will be a bit of a template for ChucK instruments in unreal 
//UCHUCK();
//UINCLUDE("ABK-HmxMidi/HmxMidi.ck");



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
hihat =>  g2 => dac;
1 => g2.gain;
0.5 => delay.gain;
// read files, we use me.dir() + "path_in_side_working_directory" syntax to get relative file paths that we can package with the plugin or a game
// DON'T double click the wav files in the project browser, it will crash unreal until I fix it 

me.dir() + "drumkit/kick-0.wav" => kick.read;
me.dir() + "drumkit/snare-0.wav" => snare.read;
me.dir() + "drumkit/hihat-0.wav" => hihat.read;

//we set the play position to the end of the file so that they don't initially play, playback will be achieved by set the position back to zero
kick.samples() => kick.pos;
snare.samples() => snare.pos;
hihat.samples() => hihat.pos;
0.3 => hihat.gain;

fun void NoteOn( int m, int v )
{
   // v * 1.0 / 128 => float velocity;
   // e.noteOn( m, velocity );

	if(m == 36)
		{
		0 => kick.pos;
		}

	if(m==38)
		{
		0 => snare.pos;
		}

	if(m==42)
		{
		0 => hihat.pos;
		}
	    //<<< "on", m, v >>>;
}


global HmxMidiIn HarmonixMidi;
MidiMsg msg;

while( true )
{
    HarmonixMidi => now;

    while(HarmonixMidi.recv(msg))
    {
       if(HarmonixMidi.IsStdNoteOn(msg))
       {
        NoteOn(msg.data2, msg.data3);
       }
       else if(HarmonixMidi.IsStdNoteOff(msg))
        {
       // NoteOff(msg.data2);

        }
    }

}
