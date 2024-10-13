// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "MetasoundExecutableOperator.h"
#include "MetasoundFacade.h"
#include "MetasoundNodeInterface.h"
#include "MetasoundParamHelper.h"
#include "MetasoundSampleCounter.h"
#include "MetasoundStandardNodesCategories.h"
#include "MetasoundNodeInterface.h"
#include "MetasoundVertex.h"

#include "HarmonixMetasound/DataTypes/MidiStream.h"
#include "HarmonixMetasound/DataTypes/MusicTransport.h"
#include "HarmonixDsp/AudioUtility.h"
#include "HarmonixMetasound/Common.h"

#include "HarmonixMetasound/MidiOps/StuckNoteGuard.h"
#include "DSP/Chorus.h"
#include "DSP/Envelope.h"
#include "DSP/Amp.h"
#include "DSP/DelayStereo.h"
#include "DSP/Chorus.h"	
#include "Engine/DataTable.h"
//#include "DSP/Array"
#include "DSP/FloatArrayMath.h"
#include "Components/SynthComponent.h"
#include "HarmonixDsp/Ramper.h"
#include "Chunreal/chuck/chuck.h"
//#include "Sfizz.h"
#include <vector>
#include "Chunreal.h"
#include "ChuckInstance.h"
//#include "MidiStreamTrackIsolatorNode.h"

//#include "SfizzSynthNode.h"
//#include "MidiTrackIsolator.h"

DEFINE_LOG_CATEGORY_STATIC(LogChucKMidiNode, VeryVerbose, All);

#define LOCTEXT_NAMESPACE "ChunrealMetasounds_ChuckMidiRenderer"

namespace ChunrealMetasounds::ChuckMidiRenderer
{
	struct FRawMidiMsg
	{
		int8 Status;
		int8 Data1;
		int8 Data2;
	};
	
	using namespace Metasound;
	using namespace HarmonixMetasound;

	const FNodeClassName& GetClassName()
	{
		static FNodeClassName ClassName
		{
			"Chunreal",
			"ChuckMidiPlayerNode",
			""
		};
		return ClassName;
	}

	int32 GetCurrentMajorVersion()
	{
		return 1;
	}

	namespace Inputs
	{
		DEFINE_INPUT_METASOUND_PARAM(Compile, "Compile", "Compiles the input ChucK code with the VM")
		DEFINE_INPUT_METASOUND_PARAM(ChuckInstance, "Chuck Instance", "Chuck Instance")
		//audio inputs
		DEFINE_INPUT_METASOUND_PARAM(AudioInLeft, "Audio In Left", "optional audio input into ChucK vm");
		DEFINE_INPUT_METASOUND_PARAM(AudioInRight, "Audio In Right", "optional audio input into ChucK vm");
		//midi input
		DEFINE_INPUT_METASOUND_PARAM(MidiStream, "MidiStream", "MidiStream");
		DEFINE_INPUT_METASOUND_PARAM(TrackIndex, "Track Index", "Track");
		DEFINE_INPUT_METASOUND_PARAM(ChannelIndex, "Channel Index", "Channel");
		DEFINE_INPUT_METASOUND_PARAM(Code, "Code", "ChucK Code")
		DEFINE_INPUT_METASOUND_PARAM(ID, "ID", "ChuckID")
		//DEFINE_INPUT_METASOUND_PARAM(IncludeConductorTrack, "Include Conductor Track", "Enable to include the conductor track (AKA track 0)");
	}

	namespace Outputs
	{
		DEFINE_OUTPUT_METASOUND_PARAM(AudioOutLeft, "Audio Out Left", "Left output of SFizz Synth");
		DEFINE_OUTPUT_METASOUND_PARAM(AudioOutRight, "Audio Out Right", "Right output of Sfizz Synth");
	}

	class FChunrealMetasoundMidiOperator final : public TExecutableOperator<FChunrealMetasoundMidiOperator>
	{
	public:
		static const FNodeClassMetadata& GetNodeInfo()
		{
			auto InitNodeInfo = []() -> FNodeClassMetadata
				{
					FNodeClassMetadata Info;
					Info.ClassName = GetClassName();
					Info.MajorVersion = 1;
					Info.MinorVersion = 0;
					Info.DisplayName = INVTEXT("Chuck Instance Renderer Node");
					Info.Description = INVTEXT("This node takes a compiled chuck instances and produces audio from it, can also pass a midi stream as events into chuck");
					Info.Author = Info.Author = TEXT("Amir Ben-Kiki");
					Info.PromptIfMissing = PluginNodeMissingPrompt;
					Info.DefaultInterface = GetVertexInterface();
					Info.CategoryHierarchy = { INVTEXT("Chunreal"), NodeCategories::Music };
					return Info;
				};

			static const FNodeClassMetadata Info = InitNodeInfo();

			return Info;
		}

		static const FVertexInterface& GetVertexInterface()
		{
			static const FVertexInterface Interface(
				FInputVertexInterface(

					TInputDataVertex<FChuckInstance>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::ChuckInstance)),
					//audio inputs
					TInputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::AudioInLeft)),
					TInputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::AudioInRight)),

					TInputDataVertex<FMidiStream>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::MidiStream)),
					TInputDataVertex<int32>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::TrackIndex), 0),
					TInputDataVertex<int32>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::ChannelIndex), 0)
	
				),
				FOutputVertexInterface(
					TOutputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(Outputs::AudioOutLeft)),
					TOutputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(Outputs::AudioOutRight))
				)
			);

			return Interface;
		}

		struct FInputs
		{
	
			FChuckInstanceReadRef ChuckInstance;
			FAudioBufferReadRef AudioInLeft;
			FAudioBufferReadRef AudioInRight;
			FMidiStreamReadRef MidiStream;
			FInt32ReadRef TrackIndex;
			FInt32ReadRef ChannelIndex;

		};


		static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults)
		{
			const FInputVertexInterfaceData& InputData = InParams.InputData;

			FInputs Inputs
			{
				//compile trigger

				InputData.GetOrCreateDefaultDataReadReference<FChuckInstance>(Inputs::ChuckInstanceName, InParams.OperatorSettings),
				//audio inputs
				InputData.GetOrConstructDataReadReference<FAudioBuffer>(Inputs::AudioInLeftName, InParams.OperatorSettings),
				InputData.GetOrConstructDataReadReference<FAudioBuffer>(Inputs::AudioInRightName, InParams.OperatorSettings),

				InputData.GetOrConstructDataReadReference<FMidiStream>(Inputs::MidiStreamName),
				InputData.GetOrCreateDefaultDataReadReference<int32>(Inputs::TrackIndexName, InParams.OperatorSettings),
				InputData.GetOrCreateDefaultDataReadReference<int32>(Inputs::ChannelIndexName, InParams.OperatorSettings)

			};

			// outputs
			FOutputVertexInterface OutputInterface;


			return MakeUnique<FChunrealMetasoundMidiOperator>(InParams, MoveTemp(Inputs));
		}

		FChunrealMetasoundMidiOperator(const FBuildOperatorParams& InParams, FInputs&& InInputs)
			: Inputs(MoveTemp(InInputs))
			, SampleRate(InParams.OperatorSettings.GetSampleRate())
			, AudioOutLeft(FAudioBufferWriteRef::CreateNew(InParams.OperatorSettings))
			, AudioOutRight(FAudioBufferWriteRef::CreateNew(InParams.OperatorSettings))
		{
			//Reset(InParams);
			//UE_LOG(LogChucKMidiNode, VeryVerbose, TEXT("Chuck Midi Synth Node Constructor"));

			//theChuck = new ChucK();
			//theChuck->setLogLevel(5);
			////Initialize Chuck params
			//theChuck->setParam(CHUCK_PARAM_SAMPLE_RATE, SampleRate);
			//theChuck->setParam(CHUCK_PARAM_INPUT_CHANNELS, 2);
			//theChuck->setParam(CHUCK_PARAM_OUTPUT_CHANNELS, 2);
			//theChuck->setParam(CHUCK_PARAM_VM_ADAPTIVE, 0);
			//theChuck->setParam(CHUCK_PARAM_VM_HALT, (t_CKINT)(false));
			////Chuck->setParam(CHUCK_PARAM_OTF_PORT, g_otf_port);
			////Chuck->setParam(CHUCK_PARAM_OTF_ENABLE, (t_CKINT)TRUE);
			////Chuck->setParam(CHUCK_PARAM_DUMP_INSTRUCTIONS, (t_CKINT)dump);
			//theChuck->setParam(CHUCK_PARAM_AUTO_DEPEND, (t_CKINT)0);
			////Chuck->setParam(CHUCK_PARAM_DEPRECATE_LEVEL, deprecate_level);
			//theChuck->setParam(CHUCK_PARAM_CHUGIN_ENABLE, true);
			////Chuck->setParam(CHUCK_PARAM_USER_CHUGINS, named_dls);
			////Chuck->setParam(CHUCK_PARAM_USER_CHUGIN_DIRECTORIES, dl_search_path);
			//theChuck->setParam(CHUCK_PARAM_IS_REALTIME_AUDIO_HINT, true);

			////Set working directory
			//FChunrealModule ChunrealModule = FModuleManager::Get().GetModuleChecked<FChunrealModule>("Chunreal");
			//theChuck->setParam(CHUCK_PARAM_WORKING_DIRECTORY, TCHAR_TO_UTF8(*ChunrealModule.workingDirectory));

	
		}

		virtual void BindInputs(FInputVertexInterfaceData& InVertexData) override
		{
			//compile trigger
			InVertexData.BindReadVertex(Inputs::ChuckInstanceName, Inputs.ChuckInstance);
			//audio inputs
			InVertexData.BindReadVertex(Inputs::AudioInLeftName, Inputs.AudioInLeft);
			InVertexData.BindReadVertex(Inputs::AudioInRightName, Inputs.AudioInRight);
			InVertexData.BindReadVertex(Inputs::MidiStreamName, Inputs.MidiStream);
			InVertexData.BindReadVertex(Inputs::TrackIndexName, Inputs.TrackIndex);
			InVertexData.BindReadVertex(Inputs::ChannelIndexName, Inputs.ChannelIndex);

		}

		virtual void BindOutputs(FOutputVertexInterfaceData& InVertexData) override
		{
			InVertexData.BindWriteVertex(Outputs::AudioOutLeftName, AudioOutLeft);
			InVertexData.BindWriteVertex(Outputs::AudioOutRightName, AudioOutRight);
		}

		void Reset(const FResetParams&)
		{
			//nullify the chuck pointer so that we can receive a new chuck instance
			delete theChuck;
			theChuck = nullptr;
			ChuckID = FString();

		}


		//destructor
		virtual ~FChunrealMetasoundMidiOperator()
		{
			UE_LOG(LogChucKMidiNode, VeryVerbose, TEXT("Chuck Midi Synth Node Destructor"));

	
			//theChuck = nullptr;

		}
			




		void HandleMidiMessage(FMidiVoiceId InVoiceId, int8 InStatus, int8 InData1, int8 InData2, int32 InEventTick, int32 InCurrentTick, float InMsOffset)
		{
			using namespace Harmonix::Midi::Constants;
			int32 Status = InStatus;
			int8 InChannel = InStatus & 0xF;
			FScopeLock Lock(&sNoteActionCritSec);
			switch (InStatus & 0xF0)
			{
			case GNoteOff:
				RawMsgData1.Add((int64) 128);
				RawMsgData2.Add((int64) InData1);
				RawMsgData3.Add((int64) InData2);
				//EpicSynth1.NoteOff(InData1);
				break;
			case GNoteOn:
				//FChunrealModule::SetChuckGlobalFloat(**Inputs.ID, "freq", (float)InData1);
				//UE_LOG(LogChucKMidiNode, VeryVerbose, TEXT("Note On: %d"), InData1);
		
				//by adressing the ChucK pointer directly we may avoid all the ID related collision
				//theChuck->globals()->setGlobalFloat("noteFreq", (float)InData1);
				//theChuck->globals()->broadcastGlobalEvent("noteEvent");
				RawMsgData1.Add((int64) 144);
				RawMsgData2.Add((int64) InData1);
				RawMsgData3.Add((int64) InData2);
		


				//EpicSynth1.NoteOn(InData1, (float) InData2);
				break;
			case GPolyPres:

		
				break;
			case GChanPres:
				break;
			case GControl:
				break;
			case GPitch:
				//UE_LOG(LogChucKMidiNode, VeryVerbose, TEXT("Pitch Bend: %d"), InData1);
				
				//PitchBendRamper.SetTarget(FMidiMsg::GetPitchBendFromData(InData1, InData2));
				break;
			}
		}


		void Execute()
		{
			const int32 BlockSizeFrames = AudioOutLeft->Num();
			PendingNoteActions.Empty();
			RawMsgData1.Empty();
			RawMsgData2.Empty();
			RawMsgData3.Empty();


			Audio::FAlignedFloatBuffer AlignedBufferLeft(*Inputs.AudioInLeft);
			Audio::FAlignedFloatBuffer AlignedBufferRight(*Inputs.AudioInRight);
			Audio::FAlignedFloatBuffer InterleavedBuffer;
			//Audio::FAlignedFloatBuffer ChuckOutputInterleavedBuffer;

			//check that we received a valid chuck through the input
			const FChuckInstance& ChuckInstance = *Inputs.ChuckInstance;
			if (!ChuckInstance.IsInitialized())
			{

				AudioOutLeft->Zero();
				AudioOutRight->Zero();

				return;
			}

			//get proxy 
			if (ChuckInstance.GetProxy()->ChuckInstance->ChuckVm != nullptr)
			{
				//pitch bend data is calculated from midi stream but we don't really pass it to chuck yet
				const float RampCallRateHz = (float)(1 / SampleRate) / (float)BlockSizeFrames;

				PitchBendRamper.SetRampTimeMs(RampCallRateHz, 5.0f);
				PitchBendRamper.SetTarget(0.0f);
				PitchBendRamper.SnapToTarget();


				CurrentTrackNumber = *Inputs.TrackIndex;
				CurrentChannelNumber = *Inputs.ChannelIndex;

			}

			theChuck = ChuckInstance.GetProxy()->ChuckInstance->ChuckVm;
			if (ChuckInstance.GetProxy()->ChuckInstance->ChuckVm == nullptr)
			{

				UE_LOG(LogChucKMidiNode, VeryVerbose, TEXT("Chuck VM is null"));

				if (bufferInitialized)
				{
					//delete DeinterleaveBuffOut;
					bufferInitialized = false;
				}
				return;
			}


			
			StuckNoteGuard.UnstickNotes(*Inputs.MidiStream, [this](const FMidiStreamEvent& Event)
				{
					//NoteOff(Event.GetVoiceId(), Event.MidiMessage.GetStdData1(), Event.MidiMessage.GetStdChannel());
				});
			


			// create an iterator for midi events in the block
			const TArray<FMidiStreamEvent>& MidiEvents = Inputs.MidiStream->GetEventsInBlock();
			auto MidiEventIterator = MidiEvents.begin();

			// create an iterator for the midi clock 
			const TSharedPtr<const FMidiClock, ESPMode::NotThreadSafe> MidiClock = Inputs.MidiStream->GetClock();


			{
				while (MidiEventIterator != MidiEvents.end())
				{

					{
						const FMidiMsg& MidiMessage = (*MidiEventIterator).MidiMessage;
						//to pass the data to chuck
				

						if (MidiMessage.IsStd()  && (*MidiEventIterator).TrackIndex == CurrentTrackNumber)
						{


							HandleMidiMessage(
								(*MidiEventIterator).GetVoiceId(),
								MidiMessage.GetStdStatus(),
								MidiMessage.GetStdData1(),
								MidiMessage.GetStdData2(),
								(*MidiEventIterator).AuthoredMidiTick,
								(*MidiEventIterator).CurrentMidiTick,
								0.0f);
						}
						else if (MidiMessage.IsAllNotesOff())
						{
							//AllNotesOff();
						}
						else if (MidiMessage.IsAllNotesKill())
						{
							//KillAllVoices();
						}
						++MidiEventIterator;
					}

				}
			}

			if (MidiClock.IsValid())
			{
				//not really working but we can pass clock data to the chuck instance

				const float ClockSpeed = MidiClock->GetSpeedAtBlockSampleFrame(0);
				//SetSpeed(ClockSpeed, !(*ClockSpeedAffectsPitchInPin));
				const float ClockTempo = MidiClock->GetTempoAtBlockSampleFrame(0);

			}

			

			FScopeLock Lock(&EpicSynth1NodeCritSection);
			//apply pitchbend

			PitchBendRamper.Ramp();

			if (!bufferInitialized)
			{
				ChuckOutputInterleavedBuffer.Reserve(BlockSizeFrames * 2);

			//	DeinterleaveBuffOut = new float* [2];
				DeinterleaveBuffOutArray.AddUninitialized(2);
			}

			//I'm not really seeing performance improvements with the interleaving methods but it's a little cleaner
			Audio::ArrayInterleave({AlignedBufferLeft, AlignedBufferRight }, InterleavedBuffer);

			if (RawMsgData1.Num() > 0)
			{
				//ProcessMidiMessage(MidiMsg);
				//set HmxData1, HmxData2, HmxData3
				theChuck->globals()->setGlobalIntArray("HmxData1", (t_CKINT*)RawMsgData1.GetData(), RawMsgData1.Num());
				theChuck->globals()->setGlobalIntArray("HmxData2", (t_CKINT*)RawMsgData2.GetData(), RawMsgData2.Num());
				theChuck->globals()->setGlobalIntArray("HmxData3", (t_CKINT*)RawMsgData3.GetData(), RawMsgData3.Num());

				//broadcast event
				theChuck->globals()->broadcastGlobalEvent("HarmonixMidi");



			}


			//Process samples by ChucK
			FChunrealModule::RunChuck(ChuckInstance.GetProxy()->ChuckInstance->ChuckVm, InterleavedBuffer.GetData(), ChuckOutputInterleavedBuffer.GetData(), BlockSizeFrames);
	
			DeinterleaveBuffOutArray[0] = AudioOutLeft->GetData();
			DeinterleaveBuffOutArray[1] = AudioOutRight->GetData();

			//theChuck->globals()->setGlobalIntArray("HmxData1", (t_CKINT*)TArray<int32>().GetData(), 0);
			//theChuck->globals()->setGlobalIntArray("HmxData2", (t_CKINT*)TArray<int32>().GetData(), 0);
			//theChuck->globals()->setGlobalIntArray("HmxData3", (t_CKINT*)TArray<int32>().GetData(), 0);


			Audio::ArrayDeinterleave(ChuckOutputInterleavedBuffer.GetData(), DeinterleaveBuffOutArray.GetData(), BlockSizeFrames, 2);

			

		}
	private:
		FInputs Inputs;
	//	FOutputs Outputs;


		struct FPendingNoteAction
		{
			int8  MidiNote = 0;
			int8  Velocity = 0;
			int32 EventTick = 0;
			int32 TriggerTick = 0;
			float OffsetMs = 0.0f;
			int32 FrameOffset = 0;
			FMidiVoiceId VoiceId;
		};

		struct FMIDINoteStatus
		{
			// is the key pressed down?
			bool KeyedOn = false;

			// is there any sound coming out of this note? (release could mean key off but voices active)
			int32 NumActiveVoices = 0;
		};


		//stuff copied from the fusion sampler...
		FCriticalSection sNoteActionCritSec;
		FCriticalSection EpicSynth1NodeCritSection;
		FCriticalSection sNoteStatusCritSec;
		static const int8 kNoteIgnore = -1;
		static const int8 kNoteOff = 0;
		static const int32 kMaxLayersPerNote = 128;
		Harmonix::Midi::Ops::FStuckNoteGuard StuckNoteGuard;

		FSampleRate SampleRate;

		//** DATA
		int32 FramesPerBlock = 0;
		int32 CurrentTrackNumber = 0;
		int32 CurrentChannelNumber = 0;
		bool MadeAudioLastFrame = false;


		TArray<int64> RawMsgData1;
		TArray<int64> RawMsgData2;
		TArray<int64> RawMsgData3;

		TArray<FPendingNoteAction> PendingNoteActions;
		FMIDINoteStatus NoteStatus[Harmonix::Midi::Constants::GMaxNumNotes];

		//pitch bend

		// on range [-1, 1]
		TLinearRamper<float> PitchBendRamper;

		// extra pitch bend in semitones
		float ExtraPitchBend = 0.0f;
		float PitchBendFactor = 1.0f;

		float FineTuneCents = 0.0f;


		
		FAudioBufferWriteRef AudioOutLeft;
		FAudioBufferWriteRef AudioOutRight;


		protected:
			//Audio::FEpicSynth1 EpicSynth1;
					//interleaved buffers

			Audio::FAlignedFloatBuffer ChuckOutputInterleavedBuffer;

			//float** DeinterleaveBuffOut;
			//do an array and we don't need to worry about deleting it
			TArray<float*> DeinterleaveBuffOutArray;

			//reference to chuck
			ChucK* theChuck = nullptr;
			FString ChuckID = FString();

			bool bufferInitialized = false;
			bool hasSporkedOnce = false;

			UChuckCode* ChuckProcessor = nullptr;
			FGuid CurrentChuckGuid;

	

	};

	class FChuckMidiRenderer final : public FNodeFacade
	{
	public:
		explicit FChuckMidiRenderer(const FNodeInitData& InInitData)
			: FNodeFacade(InInitData.InstanceName, InInitData.InstanceID, TFacadeOperatorClass<FChunrealMetasoundMidiOperator>())
		{}
		virtual ~FChuckMidiRenderer() override = default;
	};

	METASOUND_REGISTER_NODE(FChuckMidiRenderer)
}

#undef LOCTEXT_NAMESPACE // "HarmonixMetaSound"