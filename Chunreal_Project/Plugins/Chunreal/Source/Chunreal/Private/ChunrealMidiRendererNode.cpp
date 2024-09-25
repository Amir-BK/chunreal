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
#include "Components/SynthComponent.h"
#include "HarmonixDsp/Ramper.h"
//#include "Sfizz.h"
#include <vector>
#include "Chunreal.h"
//#include "MidiStreamTrackIsolatorNode.h"

//#include "SfizzSynthNode.h"
//#include "MidiTrackIsolator.h"

DEFINE_LOG_CATEGORY_STATIC(LogChucKMidiNode, VeryVerbose, All);

#define LOCTEXT_NAMESPACE "ChunrealMetasounds_ChuckMidiRenderer"

namespace ChunrealMetasounds::ChuckMidiRenderer
{
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

	class FEpic1SynthMetasoundOperator final : public TExecutableOperator<FEpic1SynthMetasoundOperator>
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
					Info.DisplayName = INVTEXT("Chuck Midi Renderer Node");
					Info.Description = INVTEXT("This nodes receives a midi stream and passes it to the chuck vm instance");
					Info.Author = PluginAuthor;
					Info.PromptIfMissing = PluginNodeMissingPrompt;
					Info.DefaultInterface = GetVertexInterface();
					Info.CategoryHierarchy = { INVTEXT("Synthesis"), NodeCategories::Music };
					return Info;
				};

			static const FNodeClassMetadata Info = InitNodeInfo();

			return Info;
		}

		static const FVertexInterface& GetVertexInterface()
		{
			static const FVertexInterface Interface(
				FInputVertexInterface(
					TInputDataVertex<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::Compile)),
					//audio inputs
					TInputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::AudioInLeft)),
					TInputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::AudioInRight)),

					TInputDataVertex<FMidiStream>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::MidiStream)),
					TInputDataVertex<int32>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::TrackIndex), 0),
					TInputDataVertex<int32>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::ChannelIndex), 0),
					TInputDataVertex<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::Code)),
					TInputDataVertex<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::ID))
	
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
			FTriggerReadRef Compile;
			FAudioBufferReadRef AudioInLeft;
			FAudioBufferReadRef AudioInRight;
			FMidiStreamReadRef MidiStream;
			FInt32ReadRef TrackIndex;
			FInt32ReadRef ChannelIndex;
			FStringReadRef Code;
			FStringReadRef ID;
		};


		static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults)
		{
			const FInputVertexInterfaceData& InputData = InParams.InputData;

			FInputs Inputs
			{
				//compile trigger
				InputData.GetOrCreateDefaultDataReadReference<FTrigger>(Inputs::CompileName, InParams.OperatorSettings),
				//audio inputs
				InputData.GetOrConstructDataReadReference<FAudioBuffer>(Inputs::AudioInLeftName, InParams.OperatorSettings),
				InputData.GetOrConstructDataReadReference<FAudioBuffer>(Inputs::AudioInRightName, InParams.OperatorSettings),

				InputData.GetOrConstructDataReadReference<FMidiStream>(Inputs::MidiStreamName),
				InputData.GetOrCreateDefaultDataReadReference<int32>(Inputs::TrackIndexName, InParams.OperatorSettings),
				InputData.GetOrCreateDefaultDataReadReference<int32>(Inputs::ChannelIndexName, InParams.OperatorSettings),
				InputData.GetOrCreateDefaultDataReadReference<FString>(Inputs::CodeName, InParams.OperatorSettings),
				InputData.GetOrCreateDefaultDataReadReference<FString>(Inputs::IDName, InParams.OperatorSettings)
			};

			// outputs
			FOutputVertexInterface OutputInterface;


			return MakeUnique<FEpic1SynthMetasoundOperator>(InParams, MoveTemp(Inputs));
		}

		FEpic1SynthMetasoundOperator(const FBuildOperatorParams& InParams, FInputs&& InInputs)
			: Inputs(MoveTemp(InInputs))
			, SampleRate(InParams.OperatorSettings.GetSampleRate())
			, AudioOutLeft(FAudioBufferWriteRef::CreateNew(InParams.OperatorSettings))
			, AudioOutRight(FAudioBufferWriteRef::CreateNew(InParams.OperatorSettings))
		{
			Reset(InParams);
		}

		virtual void BindInputs(FInputVertexInterfaceData& InVertexData) override
		{
			//compile trigger
			InVertexData.BindReadVertex(Inputs::CompileName, Inputs.Compile);
			//audio inputs
			InVertexData.BindReadVertex(Inputs::AudioInLeftName, Inputs.AudioInLeft);
			InVertexData.BindReadVertex(Inputs::AudioInRightName, Inputs.AudioInRight);
			InVertexData.BindReadVertex(Inputs::MidiStreamName, Inputs.MidiStream);
			InVertexData.BindReadVertex(Inputs::TrackIndexName, Inputs.TrackIndex);
			InVertexData.BindReadVertex(Inputs::ChannelIndexName, Inputs.ChannelIndex);
			InVertexData.BindReadVertex(Inputs::CodeName, Inputs.Code);
			InVertexData.BindReadVertex(Inputs::IDName, Inputs.ID);
		}

		virtual void BindOutputs(FOutputVertexInterfaceData& InVertexData) override
		{
			InVertexData.BindWriteVertex(Outputs::AudioOutLeftName, AudioOutLeft);
			InVertexData.BindWriteVertex(Outputs::AudioOutRightName, AudioOutRight);
		}

		void Reset(const FResetParams&)
		{
			//Create Chuck
			theChuck = new ChucK();

			//Initialize Chuck params
			theChuck->setParam(CHUCK_PARAM_SAMPLE_RATE, FChunrealModule::GetChuckSampleRate());
			theChuck->setParam(CHUCK_PARAM_INPUT_CHANNELS, 2);
			theChuck->setParam(CHUCK_PARAM_OUTPUT_CHANNELS, 2);
			theChuck->setParam(CHUCK_PARAM_VM_ADAPTIVE, 0);
			theChuck->setParam(CHUCK_PARAM_VM_HALT, (t_CKINT)(false));
			//theChuck->setParam(CHUCK_PARAM_OTF_PORT, g_otf_port);
			//theChuck->setParam(CHUCK_PARAM_OTF_ENABLE, (t_CKINT)TRUE);
			//theChuck->setParam(CHUCK_PARAM_DUMP_INSTRUCTIONS, (t_CKINT)dump);
			theChuck->setParam(CHUCK_PARAM_AUTO_DEPEND, (t_CKINT)0);
			//theChuck->setParam(CHUCK_PARAM_DEPRECATE_LEVEL, deprecate_level);
			theChuck->setParam(CHUCK_PARAM_CHUGIN_ENABLE, false);
			//theChuck->setParam(CHUCK_PARAM_USER_CHUGINS, named_dls);
			//theChuck->setParam(CHUCK_PARAM_USER_CHUGIN_DIRECTORIES, dl_search_path);
			theChuck->setParam(CHUCK_PARAM_HINT_IS_REALTIME_AUDIO, true);

			//Set log
#if PRINT_CHUCK_LOG
			theChuck->setStdoutCallback(FChunrealModule::printThisFromChuck);
			theChuck->setStderrCallback(FChunrealModule::printThisFromChuck);
			//theChuck->setLogLevel(CK_LOG_INFO);
#endif

//Start Chuck
			theChuck->init();
			theChuck->start();

			//Store ChucK reference with ID
			if (!((FString)(**Inputs.ID)).IsEmpty())
			{
				FChunrealModule::StoreChuckRef(theChuck, **Inputs.ID);
				//FChunrealModule::Log(FString("Added ChucK ID: ") + **ID);
			}
		}


		//destructor
		virtual ~FEpic1SynthMetasoundOperator()
		{
			UE_LOG(LogChucKMidiNode, VeryVerbose, TEXT("Chuck Midi Synth Node Destructor"));

			//Remove ChucK reference with ID
			if (!((FString)(**Inputs.ID)).IsEmpty())
			{
				FChunrealModule::RemoveChuckRef(**Inputs.ID);
				//FChunrealModule::Log(FString("Removed ChucK ID: ") + **ID);
			}

			//Delete allocated memory
			delete inBufferInterleaved;
			delete outBufferInterleaved;

			//Delete ChucK
			delete theChuck;
			theChuck = nullptr;

		}
			




		void HandleMidiMessage(FMidiVoiceId InVoiceId, int8 InStatus, int8 InData1, int8 InData2, int32 InEventTick, int32 InCurrentTick, float InMsOffset)
		{
			using namespace Harmonix::Midi::Constants;
			int8 InChannel = InStatus & 0xF;
			FScopeLock Lock(&sNoteActionCritSec);
			switch (InStatus & 0xF0)
			{
			case GNoteOff:
		
				//EpicSynth1.NoteOff(InData1);
				break;
			case GNoteOn:
				//FChunrealModule::SetChuckGlobalFloat(**Inputs.ID, "freq", (float)InData1);
				UE_LOG(LogChucKMidiNode, VeryVerbose, TEXT("Note On: %d"), InData1);
		
				FChunrealModule::SetChuckGlobalFloat(**Inputs.ID, "noteFreq", (float)InData1);
				FChunrealModule::BroadcastChuckGlobalEvent(**Inputs.ID, "customEvent");


				//EpicSynth1.NoteOn(InData1, (float) InData2);
				break;
			case GPolyPres:

		
				break;
			case GChanPres:
				break;
			case GControl:
				break;
			case GPitch:
				UE_LOG(LogChucKMidiNode, VeryVerbose, TEXT("Pitch Bend: %d"), InData1);
				
				//PitchBendRamper.SetTarget(FMidiMsg::GetPitchBendFromData(InData1, InData2));
				break;
			}
		}


		void Execute()
		{
			const int32 BlockSizeFrames = AudioOutLeft->Num();
			PendingNoteActions.Empty();

			const FTrigger& trigger = *Inputs.Compile;
			const float* inBufferLeft = Inputs.AudioInLeft->GetData();
			const float* inBufferRight = Inputs.AudioInRight->GetData();
			float* outBufferLeft = AudioOutLeft->GetData();
			float* outBufferRight = AudioOutLeft->GetData();
			const int32 numSamples = Inputs.AudioInLeft->Num();

			//Run ChucK code
			if (trigger.IsTriggered())
			{
				if (hasSporkedOnce)
				{
					Chuck_Msg* msg = new Chuck_Msg;
					msg->type = 3;  //MSG_REMOVEALL
					theChuck->vm()->process_msg(msg);
				}
				else
				{
					hasSporkedOnce = true;
				}
				FChunrealModule::CompileChuckCode(theChuck, TCHAR_TO_UTF8(**Inputs.Code));


				const float RampCallRateHz = (float)(1 / SampleRate) / (float)BlockSizeFrames;

				PitchBendRamper.SetRampTimeMs(RampCallRateHz, 5.0f);
				PitchBendRamper.SetTarget(0.0f);
				PitchBendRamper.SnapToTarget();



				DeinterleavedBuffer.resize(2 * BlockSizeFrames);
				DecodedAudioDataBuffer.resize(2 * BlockSizeFrames);
				//DeinterleavedBuffer[0] = AudioOutLeft->GetData();
				//DeinterleavedBuffer[1] = AudioOutRight->GetData();


				CurrentTrackNumber = *Inputs.TrackIndex;
				CurrentChannelNumber = *Inputs.ChannelIndex;
			}

			//Make interleaved buffers
			if (!bufferInitialized)
			{
				inBufferInterleaved = new float[numSamples * 2];
				outBufferInterleaved = new float[numSamples * 2];

				bufferInitialized = true;
			}
			for (int i = 0; i < numSamples; i++)
			{
				 *(inBufferInterleaved + i * 2) = *(inBufferLeft + i);
				 *(inBufferInterleaved + i * 2 + 1) = *(inBufferRight + i);
			}

			
			StuckNoteGuard.UnstickNotes(*Inputs.MidiStream, [this](const FMidiStreamEvent& Event)
				{
					//NoteOff(Event.GetVoiceId(), Event.MidiMessage.GetStdData1(), Event.MidiMessage.GetStdChannel());
				});
			
			//Filter.SetFilterValues(*Inputs.TrackIndex, *Inputs.ChannelIndex, false);

			//Outputs.MidiStream->PrepareBlock();

			// create an iterator for midi events in the block
			const TArray<FMidiStreamEvent>& MidiEvents = Inputs.MidiStream->GetEventsInBlock();
			auto MidiEventIterator = MidiEvents.begin();

			// create an iterator for the midi clock 
			const TSharedPtr<const FMidiClock, ESPMode::NotThreadSafe> MidiClock = Inputs.MidiStream->GetClock();

			int32 FramesRequired = 1;
			//while (FramesRequired > 0)
			{
				while (MidiEventIterator != MidiEvents.end())
				{

					{
						const FMidiMsg& MidiMessage = (*MidiEventIterator).MidiMessage;
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
				const float ClockSpeed = MidiClock->GetSpeedAtBlockSampleFrame(0);
				//SetSpeed(ClockSpeed, !(*ClockSpeedAffectsPitchInPin));
				const float ClockTempo = MidiClock->GetTempoAtBlockSampleFrame(0);
				//sfizz_send_bpm_tempo(SfizzSynth, 0, ClockTempo);
				//SetTempo(ClockTempo);
				//const float Beat = MidiClock->GetQuarterNoteIncludingCountIn();
				//SetBeat(Beat);
			}

			

			FScopeLock Lock(&EpicSynth1NodeCritSection);
			//apply pitchbend

			PitchBendRamper.Ramp();

			//Process samples by ChucK
			FChunrealModule::RunChuck(theChuck, (float*)inBufferInterleaved, outBufferInterleaved, numSamples);

			//Retrive each output channel and apply volume multiplier
			for (int i = 0; i < BlockSizeFrames; i++)
			{
				*(outBufferLeft + i) = *(outBufferInterleaved + i * 2);// *(*Amplitude);
				*(outBufferRight + i) = *(outBufferInterleaved + i * 2 + 1);// *(*Amplitude);
			}
			/*
			EpicSynth1.SetOscPitchBend(0, PitchBendRamper.GetCurrent());
			EpicSynth1.SetOscPitchBend(1, PitchBendRamper.GetCurrent());
			UE_LOG(LogChucKMidiNode, VeryVerbose, TEXT("Pitch Bend: %f"), PitchBendRamper.GetCurrent());

			//acquire samples from synth
			for (int32 SampleIndex = 0; SampleIndex < BlockSizeFrames; ++SampleIndex)
			{
			
				//UE_LOG(LogChucKMidiNode, VeryVerbose, TEXT("Pitch Bend: %f"), PitchBendRamper.GetCurrent());

				EpicSynth1.GenerateFrame(&DecodedAudioDataBuffer.data()[SampleIndex]);
				AudioOutLeft->GetData()[SampleIndex] = DecodedAudioDataBuffer.data()[SampleIndex];
				AudioOutRight->GetData()[SampleIndex] = DecodedAudioDataBuffer.data()[SampleIndex + 1];
			}
			*/




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

		TArray<FPendingNoteAction> PendingNoteActions;
		FMIDINoteStatus NoteStatus[Harmonix::Midi::Constants::GMaxNumNotes];

		//pitch bend

		// on range [-1, 1]
		TLinearRamper<float> PitchBendRamper;

		// extra pitch bend in semitones
		float ExtraPitchBend = 0.0f;
		float PitchBendFactor = 1.0f;

		float FineTuneCents = 0.0f;
		
		//sfizz stuff
		//sfizz_synth_t* SfizzSynth;

		std::vector<float>   DecodedAudioDataBuffer;
		std::vector<float*>  DeinterleavedBuffer;
	
		bool bSuccessLoadSFZFile = false;
		bool bEpic1SynthCreated = false;
		FString LibPath;
		FString ScalaPath;

		int32 VoiceCount = 8;

		
		FAudioBufferWriteRef AudioOutLeft;
		FAudioBufferWriteRef AudioOutRight;
		//unDAWMetasounds::TrackIsolatorOP::FMidiTrackIsolator Filter;

		protected:
			//Audio::FEpicSynth1 EpicSynth1;
					//interleaved buffers
			float* inBufferInterleaved;
			float* outBufferInterleaved;

			//reference to chuck
			ChucK* theChuck = nullptr;

			bool bufferInitialized = false;
			bool hasSporkedOnce = false;

	

	};

	class FChuckMidiRenderer final : public FNodeFacade
	{
	public:
		explicit FChuckMidiRenderer(const FNodeInitData& InInitData)
			: FNodeFacade(InInitData.InstanceName, InInitData.InstanceID, TFacadeOperatorClass<FEpic1SynthMetasoundOperator>())
		{}
		virtual ~FChuckMidiRenderer() override = default;
	};

	METASOUND_REGISTER_NODE(FChuckMidiRenderer)
}

#undef LOCTEXT_NAMESPACE // "HarmonixMetaSound"