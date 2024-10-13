//credit : © Jack Atherton 2020.  https://ccrma.stanford.edu/~lja/timbre-library/ 

//UCHUCK();
//UINSTRUMENT();
//UINCLUDE("JA-TimbreLibrary/intqueue.ck");
//UINCLUDE("JA-TimbreLibrary/voicebankvoice.ck");
//UINCLUDE("JA-TimbreLibrary/voicebank.ck");
//UINCLUDE("ABK-HmxMidi/HmxMidi.ck");

class SynthtarVoice extends VoiceBankVoice
{
    // override
    0.12 => gainAtZeroVelocity;
    0.4 => highCutoffSensitivity;
    -0.7 => lowCutoffSensitivity;
    
    2 => int unison;

    float myFreqWaver;
    400 => float myLPFCutoff;
    400 => float myHPFCutoff;
    LPF lpf1 => LPF lpf2 => adsr;
    Gain preHPF => HPF hpf => adsr;
    // 75% mix LPF to HPF
    0.25 => hpf.gain;
    0.75 => lpf1.gain;
    // but also pregain
    1.3 => preHPF.gain;
    // sane initial value to prevent CLICK CLAP BOOM 
    1300 => hpf.freq;
    
    // TODO: cutoff1+2 modulated by 0.19 by aftertouch, can't really 
    // test that without aftertouch
    
    SqrOsc osc1[unison];
    SqrOsc osc2[unison];
    SqrOsc osc3[unison];
    SinOsc root => adsr;
    0.2 => root.gain;
 
    // oscs, gain, widths
    for( int i; i < unison; i++ )
    {
        0.39 / unison => osc1[i].gain;
        0.30 / unison => osc2[i].gain;
        0.30 / unison => osc3[i].gain;
        0.9 => osc1[i].width;
        0.55 => osc2[i].width;
        0.8 => osc3[i].width;
        osc1[i] => lpf1;
        osc2[i] => lpf1;
        osc3[i] => lpf1;
        osc1[i] => preHPF;
        osc2[i] => preHPF;
        osc3[i] => preHPF;
    }
    
    
    // smoothed sample and hold
    Step lfo1 => LPF lfo1LPF => Envelope lfo1env => blackhole;
    360::ms => lfo1env.duration;
    fun void startLFO1()
    {
        1 => lfo1env.keyOn;
        5 => lfo1LPF.freq;
        while( true )
        {
            Math.random2f( -1, 1 ) => lfo1.next;
            // TODO scale and hook up
            0.03 * lfo1env.last() + 1 => myFreqWaver;
            5::ms => now;
        }
    }
    
    null @=> Shred startLFO1Shred;

    
    
    fun void sync()
    {
        for( int i; i < unison; i++ )
        {
            // TODO: necessary?
            // 0 => osc1[i].phase => osc2[i].phase;
        }
        if( startLFO1Shred != null )
        {
            startLFO1Shred.exit();
        }
        spork ~ startLFO1() @=> startLFO1Shred;
    }
    
    
    // analog: randomly alter pitch and cutoff (0 to 1: 1. units?)
    0.002 => float analog;
    float analogs[unison + 1];
    fun void calculateAnalog()
    {
        for( int i; i < analogs.size(); i++ )
        {
            Math.random2f( 1 - analog, 1 + analog ) => analogs[i];
        }
    }
    
    
    
    // cutoff
    fun float cutoffToHz( float cutoff )
    {
        return Math.min( Std.scalef( Math.pow( Std.clampf( cutoff, 0, 1 ), 3 ), 0, 1, myFreq, 18000 ), 18000 );
    }
    
    // LPF cutoff envelope
    ADSR lpfEnv => blackhole;
    0.015 => float lpfSustain;
    190::ms => dur lpfDecay;
    lpfEnv.set( 1::ms, lpfDecay, lpfSustain, 2000::ms );
    
    fun void triggerLPFEnv()
    {
        // reset
        0 => lpfEnv.value;
        lpfSustain => lpfEnv.sustainLevel;
        // bounds
        0.592 => float minCutoff;
        // higher cutoff at higher pitch and at higher velocity
        Std.scalef( Math.pow( myVelocity, 1.6 ), 0, 1, 0.6, 1.0 ) => float maxCutoff;
        
        minCutoff => float currentCutoff;
        maxCutoff - minCutoff => float cutoffDiff;
        
        Std.scalef( myVelocity, 0, 1, 1, 7.21 ) * lpfDecay => lpfEnv.decayTime;
        
        lpfEnv.keyOn( 1 );
        
        5::ms => dur delta;
                        
        while( true )
        {
            // set
            minCutoff + cutoffDiff * Math.pow( lpfEnv.value(), 3 ) => currentCutoff;
            // Math.pow( currentCutoff, 3 ) => currentCutoff;
            currentCutoff + myCutoff => this.cutoffToHz => myLPFCutoff;
                                    
            // wait
            delta => now;
        }        
    }
    spork ~ this.triggerLPFEnv() @=> Shred triggerLPFEnvShred;
    
    fun void endLPFEnv()
    {
        lpfEnv.keyOff( 1 );
    }
    
    
    ADSR hpfEnv => blackhole;
    hpfEnv.set( 5::ms, 10.3::ms, 0.0, 0::ms );
    
    fun void triggerHPFEnv()
    {
        // reset
        0 => hpfEnv.value;
        // bounds
        0.186 => float minCutoff;
        // higher cutoff at higher pitch and at higher velocity
        Std.scalef( Math.pow( myVelocity, 1.6 ), 0, 1, 0.3, 0.9 ) => float maxCutoff;
        
        minCutoff => float currentCutoff;
        maxCutoff - minCutoff => float cutoffDiff;
                
        hpfEnv.keyOn( 1 );
        
        0.5::ms => dur delta;
        
        now + hpfEnv.attackTime() + hpfEnv.decayTime() + 2::ms => time endTime;
        while( now < endTime )
        {
            // set
            minCutoff + cutoffDiff * Math.pow( hpfEnv.value(), 3 ) => currentCutoff;
            // Math.pow( currentCutoff, 3 ) => currentCutoff;
            currentCutoff + myCutoff => this.cutoffToHz => myHPFCutoff;
            Math.min( myHPFCutoff * analogs[unison], 21000 ) => hpf.freq;
            //<<< currentCutoff, myHPFCutoff >>>;
            
            // wait
            delta => now;
        }        
    }
    null @=> Shred triggerHPFEnvShred;
    
    fun void endHPFEnv()
    {
        hpfEnv.keyOff( 1 );
    }

    
    // resonances
    1.2 => lpf1.Q => lpf2.Q;
    1.9 => hpf.Q;
    
    // then ADSR on volume
    1300::ms => rTime;
    adsr.set( 1::ms, 5000::ms, 0.238, rTime );
    
    // osc1: freq
    // osc2: -4 (CBD 50%)
    // osc3: +6 (CBD 50%)
    fun void applyFreqs()
    {
        float f1, f2, f3;
        while( true )
        {
            Math.min( myLPFCutoff * analogs[unison], 21000 ) => lpf1.freq => lpf2.freq;
            
            // myFreq
            myFreq * myFreqWaver => f1;
            // -2 cents - (2 cents at C3)
            f1 * 0.998845 - 0.15108555 => f2;
            // +3 cents + (3 cents at C3)
            f1 * 1.001734 + 0.22682454 => f3;
            
            for( int i; i < unison; i++ )
            {
                f1 * analogs[i] => osc1[i].freq;
                f2 * analogs[i] => osc2[i].freq;
                f3 * analogs[i] => osc3[i].freq;
            }
            
            f1 => root.freq;

            10::ms => now;
        }
    }
    spork ~ this.applyFreqs();
    
        
    // trigger note on
    fun void noteOn()
    {
        // sync
        sync();
        // key on
        adsr.keyOn( 1 );
        triggerLPFEnvShred.exit();
        if( triggerHPFEnvShred != null ) { triggerHPFEnvShred.exit(); }
        spork ~ this.triggerLPFEnv() @=> triggerLPFEnvShred;
        spork ~ this.triggerHPFEnv() @=> triggerHPFEnvShred;
        calculateAnalog();
        
    }
    
    // trigger note off
    fun void noteOff()
    {
        adsr.keyOff( 1 );
        endLPFEnv();
        endHPFEnv();
    }
    
}


class Synthtar extends VoiceBank
{
    8 => numVoices;
    
    // voices
    SynthtarVoice myVoices[numVoices];
    // assign to superclass
    v.size( myVoices.size() );
    for( int i; i < myVoices.size(); i++ )
    {
        myVoices[i] @=> v[i];
    }
    // connect
    init( true );
}

Synthtar s => LPF l => dac;
0.3 => s.gain;

15000 => l.freq;

// knobs

1.0 => global float gCutoff;
20000 => global float gLowpass;

fun void ApplyGlobals()
{
    while( true )
    {
        10::ms => now;
        gLowpass => l.freq;
        gCutoff => s.cutoff;
    }
}
spork ~ ApplyGlobals();

// end knobs


global Event midiMessage;
global int midiCommand;
global int midiNote;
global int midiVelocity;

fun void NoteOn( int m, int v )
{
    v * 1.0 / 128 => float velocity;
    s.noteOn( m, velocity );
    //<<< "on", m, v >>>;
}


fun void NoteOff( int m )
{
    spork ~ s.noteOff( m );
    //<<< "off", m >>>;
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
        NoteOff(msg.data2);

        }
    }

}


