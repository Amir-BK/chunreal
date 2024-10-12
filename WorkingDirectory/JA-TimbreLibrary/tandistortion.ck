public class TanDistortion extends Chugen
{
    fun float tick( float in )
    {
        in * in => float square;
        in * square => float cubic;
        cubic * square => float five;
        five * square => float seven;
        return 0.5 * (in - cubic / 3 + 2 * five / 15 - 17 * seven / 315);
    }
}
