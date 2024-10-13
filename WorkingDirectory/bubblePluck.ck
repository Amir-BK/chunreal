//UCHUCK();
//UINSTRUMENT();
//UINCLUDE("ABK-HmxMidi/HmxMidi.ck");

// name: intqueue.ck
// desc: implements a queue that holds two elements, a "val" and a "voice"
//       addElem: with both value and voice
//       removeElem: by value, returns voice, or -1 if not found
//       removeOldestElem: returns voice, or -1 if not found
// author: Jack Atherton

class IntElem
{
    int val;
    int voice;
    null @=> IntElem @ next;
    null @=> IntElem @ prev;
}

class IntQueue
{
    
    null @=> IntElem head;
    null @=> IntElem tail;
    0 => int numElems;
    
    fun int size()
    {
        return numElems;
    }
    
    fun int addElem( int v, int voice )
    {
        IntElem newElem;
        v => newElem.val;
        voice => newElem.voice;
        
        if( numElems == 0 )
        {
            newElem @=> head;
        }
        else
        {
            newElem @=> tail.next;
            tail @=> newElem.prev;
        }
        newElem @=> tail;
        
        return numElems++;
    }
    
    fun int removeElem( int v )
    {
        head @=> IntElem current;
        while( current != null )
        {
            if( current.val == v )
            {
                numElems--;
                current.voice => int voice;
                if( current.prev != null )
                {
                    current.next @=> current.prev.next;
                }
                if( current.next != null )
                {
                    current.prev @=> current.next.prev;
                }
                if( current == head )
                {
                    current.next @=> head;
                }
                if( current == tail )
                {
                    current.prev @=> tail;
                }
                null @=> current.prev;
                null @=> current.next;
                
                return voice;
            }
            current.next @=> current;
        }
        
        return -1;
    }
    
    fun int removeOldestElem()
    {
        if( numElems == 0 )
        {
            return -1;
        }
        else if( numElems == 1 )
        {
            head.voice => int ret;
            numElems--;
            // <<< head.val, head.voice, "auto removed" >>>;
            null @=> head;
            null @=> tail; 
            return ret;
        }
        else
        {
            head.voice => int ret;
            // <<< head.val, head.voice, "auto removed" >>>;
            head.next @=> head;
            if( head != null )
            {
                null @=> head.prev;
            }
            numElems--;
            return ret;
        }
    }
}


// name: voicebank.ck
// desc: implements a bank of voices, finding free voices and
//       using old ones if all are taken
//       numVoices: how many voices there are
//       see connection strategy in comments below under "TODO"
//       noteOn( float midiNote, float velocity )
//               velocity is [0, 1]
//       noteOff( float midiNote )
//                turn off note that was turned on earlier
// author: Jack Atherton

// dependencies:
// Machine.add( me.dir() + "intqueue.ck" );
// Machine.add( me.dir() + "voicebankvoice.ck" );

class VoiceBank extends Chugraph
{
    8 => int numVoices;
    
    Gain myGain => outlet;
    gain( 1 );
    
    IntQueue voices;
    IntQueue voicesInterruptible;
    time lastMarkedInterruptibleTimes[32];
    
    // voices
    VoiceBankVoice @ v[0];
    
    // TODO: connect, like
    // v.size( numVoices );
    // for( int i; i < numVoices; i++ )
    // {
    //     myVoices[i] @=> v[i];
    // }
    // init( true );
    
    
    fun void init( int connect )
    {
        lastMarkedInterruptibleTimes.size( v.size() );
        if( connect )
        {
            for( int i; i < v.size(); i++ )
            {
                v[i] => myGain;
            }
        }
    }
    
    fun int findVoiceNotInUse()
    {
        for( int i; i < v.size(); i++ )
        {
            if( v[i].available() )
            {
                return i;
            }
        }

        return -1;
    }
    fun int allocateNewVoice( int note )
    {
        int which;
        if( voices.size() + voicesInterruptible.size() < numVoices )
        {
            findVoiceNotInUse() => which;
            voices.addElem( note, which );
        }
        else
        {
            //<<< "voices size:", voices.size(), "and interruptible size:", voicesInterruptible.size() >>>;
            if( voicesInterruptible.size() > 0 )
            {
                voicesInterruptible.removeOldestElem() => which;
            }
            else
            {
                voices.removeOldestElem() => which;
            }
            
            if( which < 0 )
            {
                //<<< "uh oh allocating" >>>;
            }
            else
            {
                voices.addElem( note, which );
                v[which].noteOff();
            }
        }
        
        //<<< which, "allocated for", note >>>;
        
        return which;
    }
    
    fun int markVoiceInterruptible( int note )
    {
        voices.removeElem( note ) => int which;
        if( which >= 0 )
        {
            voicesInterruptible.addElem( note, which );
            //<<< which, "transitioned to tail" >>>;
        }
        else
        {
            //<<< "uh oh marking interruptible" >>>;
        }
        return which;
    }
    
    fun int voiceDone( int note )
    {
        voicesInterruptible.removeElem( note ) => int which;
        if( which >= 0 )
        {
            // yay!
            //<<< which, "removed" >>>;
        }
        else
        {
            //<<< which, "probably already auto removed" >>>;
        }

        return -1;
    }
    
    fun float gain( float g )
    {
        g => myGain.gain;
        return g;
    }
    
    fun float cutoff( float c )
    {
        for( int i; i < v.size(); i++ )
        {
            c => v[i].cutoff;
        }
        return c;
    }
    
    fun void noteOn( float midiNote, float velocity )
    {
        // pick a voice
        allocateNewVoice( midiNote $ int ) => int which;
        
        // set and on
        velocity => v[which].velocity;
        midiNote => v[which].note;
        v[which].noteOn();
    }
    
    // wait for release to remove 
    fun void noteOff( float midiNote )
    {
        // look up voice
        markVoiceInterruptible( midiNote $ int ) => int which;
        if( which >= 0 )
        {
            // set in motion
            v[which].noteOff();
            // remember
            now => time myNoteOffTime => lastMarkedInterruptibleTimes[which];
            // wait
            v[which].adsrReleaseTime() => now;
            
            // only mark done if we were the last to mark interruptible
            if( lastMarkedInterruptibleTimes[which] == myNoteOffTime )
            {
                voiceDone( midiNote $ int ) => int successVoice;
                if( successVoice < 0 )
                {
                    //<<< "sad remove voice" >>>;
                }
            }
        }
    }
    
}


class VoiceBankVoice extends Chugraph
{
    // TODO: override me if you want behavior other than 
    // [0, 1] --> [0.5, 1]
    0.5 => float gainAtZeroVelocity;
    // how much cutoff responds below 0.5 and above 0.5
    -0.1 => float lowCutoffSensitivity;
    0.1 => float highCutoffSensitivity;
    // adsr
    200::ms => dur rTime;
    ADSR adsr => Gain theGain => outlet;
    // TODO: use rTime in your call to adsr.set( .., .., .., rTime );
    
    // trigger note on
    fun void noteOn()
    {
        // TODO: override me
        adsr.keyOn( true );
    }
    
    // trigger note off
    fun void noteOff()
    {
        // TODO: override me
        adsr.keyOff( true );
    }
    
    // variables you can use
    float myGain;
    float myMidi;
    float myFreq;
    float myVelocity;
    float myCutoff;
    
    // setter
    fun float gain( float g )
    {
        g => myGain => theGain.gain;
        return g;
    }
    
    // setter
    fun float velocity( float v )
    {
        v => myVelocity;
        // for adsr gain,
        // interpret velocity as starting at X and going to 1     
        // velocity controls adsr gain
        Std.scalef( myVelocity, 0, 1, gainAtZeroVelocity, 1 ) => adsr.gain;
        return v;
    }
    
    // setter
    fun float cutoff( float c )
    {
        Std.clampf( c, 0, 1 ) => c;
        if( c > 0.5 )
        {
            Std.scalef( c, 0.5, 1, 0.0, highCutoffSensitivity ) => myCutoff;
        }
        else
        {
            Std.scalef( c, 0.0, 0.5, lowCutoffSensitivity, 0.0 ) => myCutoff;
        }
        return c;
    }
    
    // setter
    fun float note( float m )
    {
        m => myMidi;
        myMidi => Std.mtof => myFreq;
        return m;
    }
    
    
    
    fun dur adsrReleaseTime()
    {
        return rTime;
    }
    
    fun int available()
    {
        // adsr done?
        return adsr.state() == 4;
    }
    
    fun int interruptible()
    {
        // adsr in release or done?
        return adsr.state() >= 3;
    }
}


class RetroVoice extends VoiceBankVoice
{
    // override
    0.5 => gainAtZeroVelocity;
    0.3 => highCutoffSensitivity;
    -0.65 => lowCutoffSensitivity;
    
    1 => int unison;

    float myFreqWaver;
    400 => float myLPFCutoff;
    400 => float myHPFCutoff;
    DelayA chorus => LPF lpf => adsr;
    
    SawOsc osc1[unison];
    SinOsc root => lpf;
    0.1 => root.gain;
    
    // oscs, gain, widths
    for( int i; i < unison; i++ )
    {
        1.0 / unison => osc1[i].gain;
        osc1[i] => lpf;
        osc1[i] => chorus;
    }
    
    // chorus 0.24Hz, "8%" intensity
    5::ms => chorus.max;
    SinOsc chorusLFO => blackhole;
    0.24 => chorusLFO.freq;
    0.13 => chorus.gain;
    
    fun void doChorus()
    {
        3::ms => dur baseDelay;
        
        while( true )
        {
            0.3::ms * chorusLFO.last() + baseDelay => chorus.delay;
            5::ms => now;
        }
        
    }
    spork ~ doChorus();
    
    // pitch lfo
    TriOsc lfo1 => Envelope lfo1env => blackhole;
    270::ms => lfo1env.duration;
    7.9 => lfo1.freq;
    fun void startLFO1()
    {
        1 => lfo1env.keyOn;
        -0.25 => lfo1.phase;
        while( true )
        {
            // TODO scale and hook up
            0.02 * lfo1env.last() + 1 => myFreqWaver;
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
    0.005 => float analog;
    float analogs[unison + 1];
    fun void calculateAnalog()
    {
        for( int i; i < analogs.size(); i++ )
        {
            Math.random2f( 1 - analog, 1 + analog ) => analogs[i];
        }
    }
    
    
    
    // lpf cutoff
    fun float cutoffToHz( float cutoff )
    {
        return Math.min( Std.scalef( Math.pow( Std.clampf( cutoff, 0, 1 ), 3 ), 0, 1, myFreq, 18000 ), 18000 );
    }
    
    // LPF cutoff envelope
    ADSR lpfEnv => blackhole;
    0.0001 => float lpfSustain;
    410::ms => dur lpfDecay;
    lpfEnv.set( 1::ms, lpfDecay, lpfSustain, 5400::ms );
    
    fun void triggerLPFEnv()
    {
        // reset
        0 => lpfEnv.value;
        lpfSustain => lpfEnv.sustainLevel;
        // bounds
        0.74 => float minCutoff;
        // higher cutoff at higher pitch and at higher velocity
        Std.scalef( Math.pow( myVelocity, 1.6 ), 0, 1, 0.34, 0.64 )
            + Std.scalef( myMidi, 0, 128, 0, 0.6 ) => float maxCutoff;
        
        minCutoff => float currentCutoff;
        maxCutoff - minCutoff => float cutoffDiff;
                
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
    1300::ms => rTime;
    adsr.set( 1::ms, 99::ms, 0.99, rTime );
    
    // osc1: freq
    fun void applyFreqs()
    {
        float f1;
        while( true )
        {
            Math.min( myLPFCutoff * analogs[unison], 21000 ) => lpf.freq;
            
            // myFreq
            myFreq * myFreqWaver => f1;
            
            for( int i; i < unison; i++ )
            {
                f1 * analogs[i] => osc1[i].freq;
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


class BubblePluckVoice extends VoiceBankVoice
{
    // override
    0.5 => gainAtZeroVelocity;
    0.8 => highCutoffSensitivity;
    -0.3 => lowCutoffSensitivity;
    
    1 => int unison;

    400 => float myLPFCutoff;
    1 => float myFreqModifier;
    LPF lpf => adsr;
    
    SqrOsc osc1[unison];
    SawOsc osc2[unison];
    SqrOsc osc3[unison];
    
    // oscs, gain, widths
    for( int i; i < unison; i++ )
    {
        0.56 / unison => osc1[i].gain;
        0.18 / unison => osc2[i].gain;
        0.26 / unison => osc3[i].gain;
        osc1[i] => lpf;
        osc2[i] => lpf;
        osc3[i] => lpf;
    }
    
    
    fun void sync()
    {
        for( int i; i < unison; i++ )
        {
            0 => osc1[i].phase => osc2[i].phase => osc3[i].phase;
        }
    }
    
    
    Envelope pitchEnv => blackhole;
    fun void PitchEnv()
    {
        6::ms => pitchEnv.duration;
        0 => pitchEnv.value;
        1 => pitchEnv.target;
        now + pitchEnv.duration() => time end;
        while( now < end )
        {
            Std.scalef( Math.pow( pitchEnv.value(), 4 ), 0, 1, 1, 4 ) => myFreqModifier;
            0.1::ms => now;
        }
        1 => myFreqModifier;
    }
    
    
    // cubic scale
    fun float cutoffToHz( float cutoff )
    {
        return Math.min( Std.scalef( Math.pow( Std.clampf( cutoff, 0, 1 ), 3 ), 0, 1, myFreq, 18000 ), 18000);
    }
    
    // LPF cutoff envelope
    ADSR lpfEnv => blackhole;
    0.349 => float lpfSustain;
    240::ms => dur lpfDecay;
    lpfEnv.set( 1::ms, lpfDecay, lpfSustain, 7700::ms );
    
    fun void triggerLPFEnv()
    {
        // reset
        0 => lpfEnv.value;
        lpfSustain => lpfEnv.sustainLevel;
        // bounds
        Math.pow( myVelocity, 1.6 ) => float v;
        Std.scalef( v, 0, 1, 0.05, 0.24 ) => float minCutoff;
        // higher cutoff at higher velocity
        Std.scalef( v, 0, 1, 0.3, 1.0 ) => float maxCutoff;
        
        minCutoff => float currentCutoff;
        maxCutoff - minCutoff => float cutoffDiff;
                
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
    330::ms => rTime;
    adsr.set( 1::ms, 100::ms, 0.995, rTime );
    
    // osc1: freq
    // osc2: +3
    // osc3: -3
    fun void applyFreqs()
    {
        float f1, f2, f3;
        while( true )
        {
            Math.min( myLPFCutoff, 21000 ) => lpf.freq;
            
            // myFreq
            myFreq * myFreqModifier => f1;
            // +3c
            f1 * 1.001734 => f2;
            // -3c
            f1 * 0.998269 => f3;
            
            for( int i; i < unison; i++ )
            {
                f1 => osc1[i].freq;
                f2 => osc2[i].freq;
                f3 => osc3[i].freq;
            }
            
            0.5::ms => now;
        }
    }
    spork ~ this.applyFreqs();
    
    // trigger note on
    fun void noteOn()
    {
        // key on
        adsr.keyOn( 1 );
        triggerLPFEnvShred.exit();
        spork ~ this.triggerLPFEnv() @=> triggerLPFEnvShred;
        sync();
        spork ~ PitchEnv();
    }
    
    // trigger note off
    fun void noteOff()
    {
        adsr.keyOff( 1 );
        endLPFEnv();
    }
    
}


class BubblePluck extends VoiceBank
{
    8 => numVoices;
    
    // voices
    BubblePluckVoice myVoices[numVoices];
    // assign to superclass
    v.size( myVoices.size() );
    for( int i; i < myVoices.size(); i++ )
    {
        myVoices[i] @=> v[i];
    }
    // connect
    init( true );    
}

BubblePluck b => LPF l => dac;
0.3 => b.gain;

10000 => l.freq;

// knobs
global float gReverb;
0 => global float gCutoff;
5000 => global float gLowpass;

fun void ApplyGlobals()
{
    while( true )
    {
        10::ms => now;
       
        gLowpass => l.freq;
        gCutoff => b.cutoff;
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
    b.noteOn( m, velocity );
    //<<< "on", m, v >>>;
}


fun void NoteOff( int m )
{
    spork ~ b.noteOff( m );
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



