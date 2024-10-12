// name: voicebankvoice.ck
// desc: a voice for a voice bank
//       gainAtZeroVelocity: map [0, 1] to gain [g, 1]
//       noteOn: trigger a note on (override this)
//       noteOff: trigger a note off (override this)
//       rTime: the release time for the adsr (override 
//              this and set the adsr manually)
//       myGain, myMidi, myFreq, myVelocity: 
//              convenience for you to use
//       gain, velocity, note, adsrReleaseTime, 
//              available, interruptible: to be used by voicebank
// author: Jack Atherton

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
