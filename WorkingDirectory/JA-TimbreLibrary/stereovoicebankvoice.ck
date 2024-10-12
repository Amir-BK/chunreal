// name: stereovoicebankvoice.ck
// desc: a voice for a voice bank
//       gainAtZeroVelocity: map [0, 1] to gain [g, 1]
//       noteOn: trigger a note on (override this)
//       noteOff: trigger a note off (override this)
//       rTime: the release time for the adsr (override 
//              this and set the adsr manually)
//       myGain, myMidi, myFreq, myVelocity: 
//              convenience for you to use
//       gain, velocity, note, adsrReleaseTime, 
//              available, interruptible: to be used by stereovoicebank
//       (dis)connect: use if you don't use stereovoicebank's init()
// author: Jack Atherton

public class StereoVoiceBankVoice
{
    // TODO: override me if you want behavior other than 
    // [0, 1] --> [0.5, 1]
    0.5 => float gainAtZeroVelocity;
    // how much cutoff responds below 0.5 and above 0.5
    -0.1 => float lowCutoffSensitivity;
    0.1 => float highCutoffSensitivity;
    // adsr
    200::ms => dur rTime;
    ADSR adsrL => Gain theGainL;
    ADSR adsrR => Gain theGainR;
    // TODO: use rTime in your call to adsrL.set( .., .., .., rTime );
    
    // trigger note on
    fun void noteOn()
    {
        // TODO: override me
        adsrL.keyOn( true );
        adsrR.keyOn( true );
    }
    
    // trigger note off
    fun void noteOff()
    {
        // TODO: override me
        adsrL.keyOff( true );
        adsrR.keyOff( true );
    }
    
    
    
    // variables you can use
    float myGain;
    float myMidi;
    float myFreq;
    float myVelocity;
    float myCutoff;
    
    fun void connect( UGen l, UGen r )
    {
        theGainL => l;
        theGainR => r;
    }
    
    fun void disconnect( UGen l, UGen r )
    {
        theGainL =< l;
        theGainR =< r;
    }
    
    // setter
    fun float gain( float g )
    {
        g => myGain => theGainL.gain => theGainR.gain;
        return g;
    }
    
    // setter
    fun float velocity( float v )
    {
        v => myVelocity;
        // for adsr gain,
        // interpret velocity as starting at X and going to 1     
        // velocity controls adsr gain
        Std.scalef( myVelocity, 0, 1, gainAtZeroVelocity, 1 ) => adsrL.gain => adsrR.gain;
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
        return adsrL.state() == 4;
    }
    
    fun int interruptible()
    {
        // adsr in release or done?
        return adsrL.state() >= 3;
    }
}
