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
#include "Chunreal/chuck/chuck.h"
//#include "Sfizz.h"
#include <vector>
#include "Chunreal.h"
#include "ChuckInstance.h"
//#include "MidiStreamTrackIsolatorNode.h"

//#include "SfizzSynthNode.h"
//#include "MidiTrackIsolator.h"

DEFINE_LOG_CATEGORY_STATIC(LogChuckCompilerNode, VeryVerbose, All);

#define LOCTEXT_NAMESPACE "ChunrealMetasounds_ChuckCompiler"

namespace ChunrealMetasounds::ChuckCompiler
{
	using namespace Metasound;
	using namespace HarmonixMetasound;

	const FNodeClassName& GetClassName()
	{
		static FNodeClassName ClassName
		{
			"Chunreal",
			"ChuckCompilerNode",
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
		DEFINE_INPUT_METASOUND_PARAM(ChuckInstance, "Chuck Code Asset", "Reference to a chuck code asset containing chuck code ")
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
		DEFINE_OUTPUT_METASOUND_PARAM(ChuckInstanceOut, "Chuck Instance", "Chuck Instance");
	}


	class ChunrealMetasoundMidiOperator final : public TExecutableOperator<ChunrealMetasoundMidiOperator>
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
					Info.DisplayName = INVTEXT("Chuck Midi Compiler");
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

					TInputDataVertex<FChuckProcessor>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::ChuckInstance))

	
				),
				FOutputVertexInterface(
					TOutputDataVertex<FChuckInstance>(METASOUND_GET_PARAM_NAME_AND_METADATA(Outputs::ChuckInstanceOut))
				)
			);

			return Interface;
		}

		struct FInputs
		{
	
			FChuckProcessorReadRef ChuckInstance;


		};

		struct FOutputs
		{
			FChuckInstanceWriteRef ChuckInstanceOut;
		};


		static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults)
		{
			const FInputVertexInterfaceData& InputData = InParams.InputData;
			//const FOutputVertexInterfaceData& OutputData = InParams.;

			FInputs Inputs
			{

				InputData.GetOrCreateDefaultDataReadReference<FChuckProcessor>(Inputs::ChuckInstanceName, InParams.OperatorSettings),


			};

			FOutputs Outputs
			{
				InputData.GetOrConstructDataWriteReference<FChuckInstance>(Outputs::ChuckInstanceOutName)
			};



			// outputs
			FOutputVertexInterface OutputInterface;


			return MakeUnique<ChunrealMetasoundMidiOperator>(InParams, MoveTemp(Inputs), MoveTemp(Outputs));
		}

		ChunrealMetasoundMidiOperator(const FBuildOperatorParams& InParams, FInputs&& InInputs, FOutputs&& InOutputs)
			: Inputs(MoveTemp(InInputs))
			, Outputs(MoveTemp(InOutputs))
			, SampleRate(InParams.OperatorSettings.GetSampleRate())
		{
	
		}

		virtual void BindInputs(FInputVertexInterfaceData& InVertexData) override
		{
			//compile trigger
			InVertexData.BindReadVertex(Inputs::ChuckInstanceName, Inputs.ChuckInstance);


		}

		virtual void BindOutputs(FOutputVertexInterfaceData& InVertexData) override
		{
			InVertexData.BindWriteVertex(Outputs::ChuckInstanceOutName, Outputs.ChuckInstanceOut);

		}

		void Reset(const FResetParams&)
		{
			//nullify the chuck pointer so that we can receive a new chuck instance
			delete theChuck;
			theChuck = nullptr;
			ChuckID = FString();

		}


		//destructor
		virtual ~ChunrealMetasoundMidiOperator()
		{
			UE_LOG(LogChuckCompilerNode, VeryVerbose, TEXT("Chuck Midi Synth Node Destructor"));


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
				//UE_LOG(LogChuckCompilerNode, VeryVerbose, TEXT("Note On: %d"), InData1);
		
				//by adressing the ChucK pointer directly we may avoid all the ID related collision
				theChuck->globals()->setGlobalFloat("noteFreq", (float)InData1);
				theChuck->globals()->broadcastGlobalEvent("noteEvent");



				//EpicSynth1.NoteOn(InData1, (float) InData2);
				break;
			case GPolyPres:

		
				break;
			case GChanPres:
				break;
			case GControl:
				break;
			case GPitch:
				//UE_LOG(LogChuckCompilerNode, VeryVerbose, TEXT("Pitch Bend: %d"), InData1);
				
				//PitchBendRamper.SetTarget(FMidiMsg::GetPitchBendFromData(InData1, InData2));
				break;
			}
		}


		void Execute()
		{
			//if we have a chuck code asset, compile it and create a chuck instance
			if (Inputs.ChuckInstance->IsInitialized() && !bIsProxySet)
			{
				auto Proxy = Inputs.ChuckInstance->GetProxy();
				//if we have a chuck code asset, compile it and create a chuck instance
				auto* NewInstantiation = Proxy->ChuckProcessor->SpawnChuckInstance(SampleRate, 2);
				Audio::FProxyDataInitParams InitParams;
				auto NewProxy = FChuckInstance(NewInstantiation->CreateProxyData(InitParams));
				//NewProxy.ChuckInstance = NewInstantiation;
				Outputs.ChuckInstanceOut->operator=(NewProxy);


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



				bIsProxySet = true;
			}

		}
	private:

		bool bIsProxySet = false;

		FInputs Inputs;
		FOutputs Outputs;


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

		//unDAWMetasounds::TrackIsolatorOP::FMidiTrackIsolator Filter;

		protected:
			//Audio::FEpicSynth1 EpicSynth1;
					//interleaved buffers
			float* inBufferInterleaved;
			float* outBufferInterleaved;

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
			: FNodeFacade(InInitData.InstanceName, InInitData.InstanceID, TFacadeOperatorClass<ChunrealMetasoundMidiOperator>())
		{}
		virtual ~FChuckMidiRenderer() override = default;
	};

	METASOUND_REGISTER_NODE(FChuckMidiRenderer)
}

#undef LOCTEXT_NAMESPACE // "HarmonixMetaSound"