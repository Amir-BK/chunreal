// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MetasoundFacade.h"
#include "MetasoundExecutableOperator.h"
#include "MetasoundNodeRegistrationMacro.h"
#include "MetasoundPrimitives.h"
#include "MetasoundStandardNodesNames.h"
#include "MetasoundTrigger.h"
#include "Internationalization/Text.h"
#include "MetasoundParamHelper.h"
#include "ChuckInstance.h"
#include <MetasoundFrontendRegistryContainer.h>
using namespace Metasound;

#define LOCTEXT_NAMESPACE "ChunrealMetasound_ChunrealValueNode"

namespace ChunrealMetasound
{
	namespace MetasoundValueNodePrivate
	{
		FNodeClassMetadata CreateNodeClassMetadata(const FName& InDataTypeName, const FName& InOperatorName, const FText& InDisplayName, const FText& InDescription, const FVertexInterface& InDefaultInterface);
	}

	namespace ValueVertexNames
	{
		METASOUND_PARAM(ChuckInstance, "Chuck Instance", "The Chuck instance to use for this node.");
		METASOUND_PARAM(ParamName, "Param Name", "The name of the parameter to set.");
		METASOUND_PARAM(InputSetTrigger, "Set", "Trigger to write the set value to the output.");
		METASOUND_PARAM(InputTargetValue, "Target Value", "Value to immediately set the output to when triggered.");
		METASOUND_PARAM(OutputOnSet, "On Set", "Triggered when the set input is triggered.");
		METASOUND_PARAM(OutputOnReset, "On Reset", "Triggered when the reset input is triggered.");
		METASOUND_PARAM(OutputValue, "Output Value", "The current output value.");
	}

	template<typename ValueType>
	class TChunrealValueOperator : public TExecutableOperator<TChunrealValueOperator<ValueType>>
	{
	public:
		static const FVertexInterface& GetDefaultInterface()
		{
			using namespace ValueVertexNames;

			static const FVertexInterface DefaultInterface(
				FInputVertexInterface(
					TInputDataVertex<FChuckInstance>(METASOUND_GET_PARAM_NAME_AND_METADATA(ChuckInstance)),
					TInputDataVertex<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(ParamName)),
					TInputDataVertex<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(InputSetTrigger)),
					TInputDataVertex<ValueType>(METASOUND_GET_PARAM_NAME_AND_METADATA(InputTargetValue))
				),
				FOutputVertexInterface(
				
				)
			);

			return DefaultInterface;
		}

		static const FNodeClassMetadata& GetNodeInfo()
		{
			auto CreateNodeClassMetadata = []() -> FNodeClassMetadata
			{
				const FName DataTypeName = GetMetasoundDataTypeName<ValueType>();
				const FName OperatorName = "Chunreal Value";
				const FText NodeDisplayName = METASOUND_LOCTEXT_FORMAT("ValueDisplayNamePattern", "Chuck Value Setter ({0})", GetMetasoundDataTypeDisplayText<ValueType>());
				const FText NodeDescription = METASOUND_LOCTEXT("ValueDescription", "Allows setting a value to output on trigger.");
				const FVertexInterface NodeInterface = GetDefaultInterface();

				return MetasoundValueNodePrivate::CreateNodeClassMetadata(DataTypeName, OperatorName, NodeDisplayName, NodeDescription, NodeInterface);
			};

			static const FNodeClassMetadata Metadata = CreateNodeClassMetadata();
			return Metadata;
		}

		static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults)
		{
			using namespace ValueVertexNames;

			const FInputVertexInterfaceData& InputData = InParams.InputData;
			
			FChuckInstanceReadRef ChuckInstance = InputData.GetOrCreateDefaultDataReadReference<FChuckInstance>(METASOUND_GET_PARAM_NAME(ChuckInstance), InParams.OperatorSettings);
			FStringReadRef ParamName = InputData.GetOrCreateDefaultDataReadReference<FString>(METASOUND_GET_PARAM_NAME(ParamName), InParams.OperatorSettings);
			FTriggerReadRef SetTrigger = InputData.GetOrConstructDataReadReference<FTrigger>(METASOUND_GET_PARAM_NAME(InputSetTrigger), InParams.OperatorSettings);


			TDataReadReference<ValueType> TargetValue = InputData.GetOrCreateDefaultDataReadReference<ValueType>(METASOUND_GET_PARAM_NAME(InputTargetValue), InParams.OperatorSettings);

			return MakeUnique<TChunrealValueOperator<ValueType>>(InParams.OperatorSettings, ChuckInstance, ParamName, SetTrigger, TargetValue);
		}


		TChunrealValueOperator(const FOperatorSettings& InSettings, const FChuckInstanceReadRef& InChuckInstance, FStringReadRef& InParamName, const FTriggerReadRef& InSetTrigger, const TDataReadReference<ValueType>& InTargetValue)
			: ChuckInstance(InChuckInstance)
			, ParamName(InParamName)
			, SetTrigger(InSetTrigger)
			, TargetValue(InTargetValue)

		{
			//*OutputValue = *InitValue;
		}

		virtual ~TChunrealValueOperator() = default;


		virtual void BindInputs(FInputVertexInterfaceData& InOutVertexData) override
		{
			using namespace ValueVertexNames;
			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(ChuckInstance), ChuckInstance);
			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(ParamName), ParamName);
			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InputSetTrigger), SetTrigger);

			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InputTargetValue), TargetValue);
		}

		virtual void BindOutputs(FOutputVertexInterfaceData& InOutVertexData) override
		{
			using namespace ValueVertexNames;

		}

		virtual FDataReferenceCollection GetInputs() const override
		{
			// This should never be called. Bind(...) is called instead. This method
			// exists as a stop-gap until the API can be deprecated and removed.
			checkNoEntry();
			return {};
		}

		virtual FDataReferenceCollection GetOutputs() const override
		{
			// This should never be called. Bind(...) is called instead. This method
			// exists as a stop-gap until the API can be deprecated and removed.
			checkNoEntry();
			return {};
		}



		void Execute()
		{
//			TriggerOnReset->AdvanceBlock();
		//	TriggerOnSet->AdvanceBlock();

			//if (*ResetTrigger)
			{
				//*OutputValue = *InitValue;
			}

			if (*SetTrigger)
			{
				//*OutputValue = *TargetValue;
			}

			//ResetTrigger->ExecuteBlock(
			//	[&](int32 StartFrame, int32 EndFrame)
			//	{
			//	},
			//	[this](int32 StartFrame, int32 EndFrame)
			//	{
			//		TriggerOnReset->TriggerFrame(StartFrame);
			//	}
			//);

			SetTrigger->ExecuteBlock(
				[&](int32 StartFrame, int32 EndFrame)
				{
				},
				[this](int32 StartFrame, int32 EndFrame)
				{
				//	TriggerOnSet->TriggerFrame(StartFrame);
				}
			);

		}

		void Reset(const IOperator::FResetParams& InParams)
		{
		//	TriggerOnSet->Reset();
			//TriggerOnReset->Reset();
			//*OutputValue = *InitValue;
		}

	private:

		TDataReadReference<FChuckInstance> ChuckInstance;
		TDataReadReference<FString> ParamName;
		TDataReadReference<FTrigger> SetTrigger;


		TDataReadReference<ValueType> TargetValue;
		//TDataWriteReference<ValueType> OutputValue;

	};

	/** TValueNode
	 *
	 *  Generates a random float value when triggered.
	 */
	template<typename ValueType>
	class CHUNREAL_API TChunrealValueNode : public FNodeFacade
	{
	public:
		/**
		 * Constructor used by the Metasound Frontend.
		 */
		TChunrealValueNode(const FNodeInitData& InInitData)
			: FNodeFacade(InInitData.InstanceName, InInitData.InstanceID, TFacadeOperatorClass<TChunrealValueOperator<ValueType>>())
		{}

		virtual ~TChunrealValueNode() = default;
	};
} // namespace Metasound

#undef LOCTEXT_NAMESPACE
