
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

public class VoiceBank extends Chugraph
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


public class VoiceBankVoice extends Chugraph
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
