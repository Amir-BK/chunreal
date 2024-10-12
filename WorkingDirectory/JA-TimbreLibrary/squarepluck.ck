class SquarePluckVoice extends VoiceBankVoice
{
    // override
    0.5 => gainAtZeroVelocity;
    0.3 => highCutoffSensitivity;
    -0.55 => lowCutoffSensitivity;
    
    2 => int unison;

    400 => float myLPFCutoff;
    LPF lpf => adsr;
    
    SqrOsc osc1[unison];
    
    // oscs, gain, widths
    for( int i; i < unison; i++ )
    {
        1.0 / unison => osc1[i].gain;
        osc1[i] => lpf;
    }
    

    
    
    fun void sync()
    {
        for( int i; i < unison; i++ )
        {
            // TODO: necessary?
            0 => osc1[i].phase;
        }
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
    
    

    fun float cutoffToHz( float cutoff )
    {
        return Math.min( Std.scalef( Math.pow( Std.clampf( cutoff, 0, 1 ), 3 ), 0, 1, myFreq, 18000 ), 18000 );
    }
    
    // LPF cutoff envelope 
    ADSR lpfEnv => blackhole;
    0.209 => float lpfSustain;
    730::ms => dur lpfDecay;
    lpfEnv.set( 1::ms, lpfDecay, lpfSustain, 820::ms );
    
    fun void triggerLPFEnv()
    {
        // reset
        0 => lpfEnv.value;
        lpfSustain => lpfEnv.sustainLevel;
        // bounds
        Math.pow( myVelocity, 1.6 ) => float v;
        0.2 + Std.scalef( myMidi, 0, 128, 0, 0.25 ) 
              + Std.scalef( v, 0, 1, 0, 0.35 ) => float minCutoff;
        Std.scalef( v, 0, 1, 0.25, 0.5 ) => float cutoffDiff;
        minCutoff => float currentCutoff;
        
        //<<< minCutoff, minCutoff + cutoffDiff >>>;
                
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
    

    
    // resonances
    1.0 => lpf.Q;
    
    // then ADSR on volume
    440::ms => rTime;
    adsr.set( 1::ms, 99::ms, 0.99, rTime );
    
    // osc1: freq
    fun void applyFreqs()
    {
        while( true )
        {
            Math.min( myLPFCutoff * analogs[unison], 21000 ) => lpf.freq;
            
            for( int i; i < unison; i++ )
            {
                myFreq * analogs[i] => osc1[i].freq;
            }

            5::ms => now;
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
        spork ~ this.triggerLPFEnv() @=> triggerLPFEnvShred;
        calculateAnalog();
        
    }
    
    // trigger note off
    fun void noteOff()
    {
        adsr.keyOff( 1 );
        endLPFEnv();
    }
        
}


class SquarePluck extends VoiceBank
{
    8 => numVoices;
    
    // voices
    SquarePluckVoice myVoices[numVoices];
    // assign to superclass
    v.size( myVoices.size() );
    for( int i; i < myVoices.size(); i++ )
    {
        myVoices[i] @=> v[i];
    }
    // connect
    init( true );
}

SquarePluck s => LPF l => JCRev rev => dac;
0.3 => s.gain;
0.0 => rev.mix;
15000 => l.freq;

// knobs
global float gReverb;
global float gCutoff;
global float gLowpass;

fun void ApplyGlobals()
{
    while( true )
    {
        10::ms => now;
        gReverb => rev.mix;
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


while( true )
{
    midiMessage => now;
    if( midiCommand >= 144 && midiCommand < 160 )
    {
        if( midiVelocity > 0 )
        {
            NoteOn( midiNote, midiVelocity );
        }
        else
        {
            NoteOff( midiNote );
        }
    }
    else if( midiCommand >= 128 && midiCommand < 144 )
    {
        NoteOff( midiNote );
    }
    else
    {
        //<<< "unknown midi command:", midiCommand, midiNote, midiVelocity >>>;
    }
}


