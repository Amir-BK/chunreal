
#include "CoreMinimal.h"
#include "MetasoundExecutableOperator.h"
#include "MetasoundFacade.h"
#include "MetasoundNodeInterface.h"
#include "MetasoundParamHelper.h"
#include "MetasoundSampleCounter.h"

#include "MetasoundStandardNodesNames.h"
#include "MetasoundTrigger.h"
#include "MetasoundVertex.h"

#include "Chunreal.h"
#include "ChuckInstance.h"

//nodes that can translate triggers to chuck events and the other way around

#define LOCTEXT_NAMESPACE "ChunrealMetasound_ChunrealTriggerEventNode"

namespace ChunrealMetasounds::ChuckEventNodes
{
	using namespace Metasound;

	namespace Inputs
	{
		METASOUND_PARAM(TriggerEvent, "Trigger", "Trigger to translate to a chuck event.");
		METASOUND_PARAM(ChuckEvent, "Chuck Event", "Chuck event to translate to a trigger.");
		METASOUND_PARAM(ChuckInstance, "Chuck Instance", "Chuck instance to use for the chuck event.");
	}

	namespace Outputs
	{
		METASOUND_PARAM(OnChuckEvent, "On Chuck Event", "Triggered when the chuck event input is triggered.");
	}

	const FNodeClassName& GetEventToTriggerNodeName()
	{
		static FNodeClassName ClassName
		{
			"Chunreal",
			"ChuckEventToTrigger",
			""
		};
		return ClassName;
	}

	const FNodeClassName& GetTriggerToEventNodeName()
	{
		static FNodeClassName ClassName
		{
			"Chunreal",
			"TriggerToChuckEvent",
			""
		};
		return ClassName;
	}

	int32 GetCurrentMajorVersion()
	{
		return 1;
	}

	int32 GetCurrentMinorVersion()
	{
		return 0;
	}

	class FChuckEventToTriggerOperator : public TExecutableOperator<FChuckEventToTriggerOperator>
	{

	public:
		static const FVertexInterface& GetDefaultInterface()
		{
			using namespace Inputs;
			using namespace Outputs;

			static const FVertexInterface DefaultInterface(
				FInputVertexInterface(
					TInputDataVertex<FChuckInstance>(METASOUND_GET_PARAM_NAME_AND_METADATA(ChuckInstance)),
					TInputDataVertex<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(ChuckEvent))
				),
				FOutputVertexInterface(
					TOutputDataVertex<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(OnChuckEvent))
				)
			);

			return DefaultInterface;
		}

		static const FNodeClassMetadata& GetNodeInfo()
		{
			auto CreateNodeClassMetadata = []() -> FNodeClassMetadata
				{
					FNodeClassMetadata Info;
					Info.ClassName = GetEventToTriggerNodeName();
					Info.MajorVersion = GetCurrentMajorVersion();
					Info.MinorVersion = GetCurrentMinorVersion();
					Info.DisplayName = LOCTEXT("ChuckEventToTriggerDisplayName", "Chuck Event to Trigger");
					Info.Description = LOCTEXT("ChuckEventToTriggerDescription", "Translates a chuck event to a trigger.");
					Info.DefaultInterface = GetDefaultInterface();
					Info.Author = TEXT("Amir Ben-Kiki");
					Info.PromptIfMissing = PluginNodeMissingPrompt;
					Info.CategoryHierarchy = { INVTEXT("Chunreal") };
					return Info;
				};

			static const FNodeClassMetadata Metadata = CreateNodeClassMetadata();
			return Metadata;
		};

		struct FInputs
		{
			FChuckInstanceReadRef ChuckInstance;
			FStringReadRef ChuckEvent;
		};

		struct FOutputs
		{
			FTriggerWriteRef OnChuckEvent;
		};

		static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults)
		{
			using namespace Inputs;
			using namespace Outputs;

			const FInputVertexInterfaceData& InputData = InParams.InputData;

			FInputs Inputs{
			InputData.GetOrCreateDefaultDataReadReference<FChuckInstance>(METASOUND_GET_PARAM_NAME(ChuckInstance), InParams.OperatorSettings),
			InputData.GetOrCreateDefaultDataReadReference<FString>(METASOUND_GET_PARAM_NAME(ChuckEvent), InParams.OperatorSettings)

			};

			//FOutputs Outputs{
			//	TDataWriteReference<FTrigger>::CreateNew(InParams.OperatorSettings)
			//};


			return MakeUnique<FChuckEventToTriggerOperator>(InParams, MoveTemp(Inputs)); //, MoveTemp(Outputs));
		}

		FChuckEventToTriggerOperator(const FBuildOperatorParams& InParams, FInputs&& InInputs) //, FOutputs&& InOutputs)
			: Inputs(MoveTemp(InInputs))
			, OnChuckEvent(TDataWriteReference<FTrigger>::CreateNew(InParams.OperatorSettings))
		{
			EventDelegate.BindRaw(this, &FChuckEventToTriggerOperator::OnReceiveGlobalEvent);
		}

		void OnReceiveGlobalEvent(const FString& InEventName) 
		{

			//so this really exposes the current limitations of this integration, Chuck events will be 'block resolution', not sample resolution
			bShouldExecuteTriggerInBlock = true;

		}

		virtual void BindInputs(FInputVertexInterfaceData& InOutVertexData) override
		{
			using namespace Inputs;
			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(ChuckInstance), Inputs.ChuckInstance);
			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(ChuckEvent), Inputs.ChuckEvent);
		}

		virtual void BindOutputs(FOutputVertexInterfaceData& InOutVertexData) override
		{
			using namespace Outputs;
			InOutVertexData.BindWriteVertex(METASOUND_GET_PARAM_NAME(OnChuckEvent), OnChuckEvent);
		}

		//@TODO cleanup and de-registration of events 
		virtual void Execute()
		{
			using namespace Inputs;


			OnChuckEvent->AdvanceBlock();
			if (bShouldExecuteTriggerInBlock)
			{
				OnChuckEvent->TriggerFrame(0);
				bShouldExecuteTriggerInBlock = false;

			}

		
			//is chuck instance proxy valid?
			if (!Inputs.ChuckInstance->IsInitialized())
			{
				//if not initialized if we should check if our chuck instance is valid, deregister event and null it
				if (CurrentChuckInstance)
				{
					//CurrentChuckInstance->UnregisterEvent(CurrentEventName);
					CurrentChuckInstance = nullptr;
				};
				
				return;
			}

			//get chuck instance from proxy
			CurrentChuckInstance = Inputs.ChuckInstance->GetProxy()->ChuckInstance;
			
			//if input event name is different from the current event name, deregister the current event
			if (CurrentEventName != *Inputs.ChuckEvent)
			{
				//if we have a current event name, deregister it
				if (!CurrentEventName.IsEmpty())
				{
					//CurrentChuckInstance->UnregisterEvent(CurrentEventName);
				}
				CurrentEventName = *Inputs.ChuckEvent;
			}

			if (CurrentEventName.IsEmpty()) return;

			//to only register once we make sure that our current event id is invalid, when deregistering in the above block we should invalidate it
			if (CurrentEventID == INDEX_NONE)
			{
				CurrentEventID = CurrentChuckInstance->SubscribeToGlobalEventNative(CurrentEventName, EventDelegate);
				//register event, in theory, we should also unregister the old one 

			}


		}

	private:

		FInputs Inputs;
		//FOutputs Outputs;
		FTriggerWriteRef OnChuckEvent;
		
		FString CurrentEventName = FString();
		UChuckInstantiation* CurrentChuckInstance = nullptr;

		FOnGlobalEventExecutedNative EventDelegate;
		int CurrentEventID = INDEX_NONE;
		bool bShouldExecuteTriggerInBlock = false;

	}; //End of FChuckEventToTriggerOperator

	class FTriggerToChuckEventOperator : public TExecutableOperator<FTriggerToChuckEventOperator>
	{

	public:

		static const FVertexInterface& GetDefaultInterface()
		{

			using namespace Inputs;
			using namespace Outputs;

			static const FVertexInterface DefaultInterface(
				FInputVertexInterface(
					TInputDataVertex<FChuckInstance>(METASOUND_GET_PARAM_NAME_AND_METADATA(ChuckInstance)),
					TInputDataVertex<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(ChuckEvent)),
					TInputDataVertex<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(TriggerEvent))
				),
				FOutputVertexInterface()
			);

			return DefaultInterface;
		}



		static const FNodeClassMetadata& GetNodeInfo()
		{
			auto CreateNodeClassMetadata = []() -> FNodeClassMetadata
				{
					FNodeClassMetadata Info;
					Info.ClassName = GetTriggerToEventNodeName();
					Info.MajorVersion = GetCurrentMajorVersion();
					Info.MinorVersion = GetCurrentMinorVersion();
					Info.DisplayName = LOCTEXT("TriggerToChuckEventDisplayName", "Trigger to Chuck Event");
					Info.Description = LOCTEXT("TriggerToChuckEventDescription", "Translates a trigger to a chuck event.");
					Info.DefaultInterface = GetDefaultInterface();
					Info.Author = TEXT("Amir Ben-Kiki");
					Info.PromptIfMissing = PluginNodeMissingPrompt;
					Info.CategoryHierarchy = { INVTEXT("Chunreal") };
					return Info;
				};

			static const FNodeClassMetadata Metadata = CreateNodeClassMetadata();
			return Metadata;
		};

		struct FInputs
		{
			FChuckInstanceReadRef ChuckInstance;
			FStringReadRef ChuckEvent;
			FTriggerReadRef TriggerEvent;
		};

		// no outputs

		static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults)
		{
			using namespace Inputs;
			using namespace Outputs;

			const FInputVertexInterfaceData& InputData = InParams.InputData;

			FInputs Inputs{
				InputData.GetOrCreateDefaultDataReadReference<FChuckInstance>(METASOUND_GET_PARAM_NAME(ChuckInstance), InParams.OperatorSettings),
				InputData.GetOrCreateDefaultDataReadReference<FString>(METASOUND_GET_PARAM_NAME(ChuckEvent), InParams.OperatorSettings),
				InputData.GetOrCreateDefaultDataReadReference<FTrigger>(METASOUND_GET_PARAM_NAME(TriggerEvent), InParams.OperatorSettings)
			};

			return MakeUnique<FTriggerToChuckEventOperator>(InParams, MoveTemp(Inputs));
		}

		FTriggerToChuckEventOperator(const FBuildOperatorParams& InParams, FInputs&& InInputs)
			: Inputs(MoveTemp(InInputs))
		{
		}

		virtual void BindInputs(FInputVertexInterfaceData& InOutVertexData) override
		{
			using namespace Inputs;
			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(ChuckInstance), Inputs.ChuckInstance);
			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(ChuckEvent), Inputs.ChuckEvent);
			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(TriggerEvent), Inputs.TriggerEvent);
		}

		virtual void BindOutputs(FOutputVertexInterfaceData& InOutVertexData) override
		{
			// no outputs
		}

		virtual void Execute()
		{
		
			//is chuck instance proxy valid?
			if (!Inputs.ChuckInstance->IsInitialized())
			{
				return;
			}

			//get chuck instance from proxy
			UChuckInstantiation* ChuckInstance = Inputs.ChuckInstance->GetProxy()->ChuckInstance;

			//get chuck event name
			FString ChuckEventName = *Inputs.ChuckEvent;

			//get trigger event
			FTrigger TriggerEvent = *Inputs.TriggerEvent;

			//if the trigger is active, send the chuck event
			if (TriggerEvent.IsTriggeredInBlock())
			{
				ChuckInstance->ExecuteGlobalEvent(ChuckEventName);
			}
		}

		FInputs Inputs;
	};

	class FChuckEventToTriggerNode final : public FNodeFacade

	{
	public:
		explicit FChuckEventToTriggerNode(const FNodeInitData& InInitData)
			: FNodeFacade(InInitData.InstanceName, InInitData.InstanceID, TFacadeOperatorClass<FChuckEventToTriggerOperator>())
		{
		}

		virtual ~FChuckEventToTriggerNode() = default;

	};
	
	METASOUND_REGISTER_NODE(FChuckEventToTriggerNode);

	class FTriggerToChuckEventNode final : public FNodeFacade
	{
	public:
		explicit FTriggerToChuckEventNode(const FNodeInitData& InInitData)
			: FNodeFacade(InInitData.InstanceName, InInitData.InstanceID, TFacadeOperatorClass<FTriggerToChuckEventOperator>())
		{
		}

		virtual ~FTriggerToChuckEventNode() = default;
	};

	METASOUND_REGISTER_NODE(FTriggerToChuckEventNode);

};