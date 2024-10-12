public class WaveTable extends Chubgraph
{
    SndBuf table => outlet;
    1 => float myBaseFreq;
    float myFreq;
    
    fun void init( string filename, int midiNote )
    {
        filename => table.read;
        1 => table.loop;
        midiNote => Std.mtof => myBaseFreq;
    }
    
    fun float gain( float g )
    {
        g => table.gain;
        return g;
    }
    
    fun float freq( float f )
    {
        f => myFreq;
        myFreq / myBaseFreq => table.rate;
        return f;
    }
    
    fun float gain() 
    { 
        return table.gain(); 
    }
    
    fun float freq()
    {
        return myFreq;
    }
    
    fun float phase( float p )
    {
        p => table.phase;
        return p;
    }
}
