// Helper class to work with harmonix midi stream

	[0] @=> global int HmxData1[];
	[0] @=> global int HmxData2[];
	[0] @=> global int HmxData3[];



144 => global float NoteOnValue;

public class HmxMidiIn extends Event
{


	fun int recv(MidiMsg msg)
	{

		if(HmxData1.size() > 0)
		{

			HmxData1[0]  => msg.data1;
			HmxData2[0]  => msg.data2;
			HmxData3[0]  => msg.data3;
			
			//popfront all three;
			HmxData1.popFront();
			HmxData2.popFront();
			HmxData3.popFront();


			return true;
		}


		return false;
	}

	fun int IsStdNoteOn(MidiMsg msg)
	{
		if (msg.data1 == 144)
		{
			if(msg.data3 > 0)
			{
			   return true;
			}

		}

		return false;
	}


	fun int IsStdNoteOff(MidiMsg msg)
	{
		if (msg.data1 == 128)
		{
			return true;
		}

		if (msg.data1 == 144)
		{
			if(msg.data3 == 0)
			{
			   return true;
			}

		}

		return false;


	}
}