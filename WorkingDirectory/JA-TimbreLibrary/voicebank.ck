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
			return true;
        }
        else
        {
            //<<< which, "probably already auto removed" >>>;
			return false;
        }


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
