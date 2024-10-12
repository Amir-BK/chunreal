public class AbsDistortion extends Chugen
{
    fun float tick( float in )
    {
        return 1.9 * in / ( 1 + Math.fabs( in ) );
    }
}