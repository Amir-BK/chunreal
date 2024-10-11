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
#include "Chunreal/chuck/chuck_oo.h"
#include "MetasoundParamHelper.h"
#include "ChuckInstance.h"
#include <MetasoundFrontendRegistryContainer.h>
#include <Chunreal/chuck/chuck_oo.h>
using namespace Metasound;

#define LOCTEXT_NAMESPACE "ChunrealMetasound_ChunrealValueNode"

namespace ChunrealMetasound
{
		
	namespace ChuckValueNodePrivate
	{
		FNodeClassMetadata CreateSetterNodeClassMetadata(const FName& InDataTypeName, const FName& InOperatorName, const FText& InDisplayName, const FText& InDescription, const FVertexInterface& InDefaultInterface);
		FNodeClassMetadata CreateGetterNodeClassMetadata(const FName& InDataTypeName, const FName& InOperatorName, const FText& InDisplayName, const FText& InDescription, const FVertexInterface& InDefaultInterface);
	
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
	class TChunrealValueGetterOperator : public TExecutableOperator<TChunrealValueGetterOperator<ValueType>>
	{
	public:
		static const FVertexInterface& GetDefaultInterface()
		{
			using namespace ValueVertexNames;

			static const FVertexInterface DefaultInterface(
				FInputVertexInterface(
					TInputDataVertex<FChuckInstance>(METASOUND_GET_PARAM_NAME_AND_METADATA(ChuckInstance)),
					TInputDataVertex<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(ParamName))
					//initial value
				
				),
				FOutputVertexInterface(
					TOutputDataVertex<ValueType>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutputValue))
				)
			);

			return DefaultInterface;

		}
		static const FNodeClassMetadata& GetNodeInfo()
		{
			auto CreateNodeClassMetadata = []() -> FNodeClassMetadata
				{
					const FName DataTypeName = GetMetasoundDataTypeName<ValueType>();
					const FName OperatorName = "Chunreal Value Getter";
					const FText NodeDisplayName = METASOUND_LOCTEXT_FORMAT("ValueDisplayNamePattern", "Chuck Value Getter ({0})", GetMetasoundDataTypeDisplayText<ValueType>());
					const FText NodeDescription = METASOUND_LOCTEXT("ValueDescription", "Allows getting a value from Chuck.");
					const FVertexInterface NodeInterface = GetDefaultInterface();

					return ChuckValueNodePrivate::CreateGetterNodeClassMetadata(DataTypeName, OperatorName, NodeDisplayName, NodeDescription, NodeInterface);
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
			
			//output value
			TDataWriteReference<ValueType> OutputValue = InputData.GetOrConstructDataWriteReference<ValueType>(METASOUND_GET_PARAM_NAME(OutputValue));

			return MakeUnique<TChunrealValueGetterOperator<ValueType>>(InParams.OperatorSettings, ChuckInstance, ParamName, OutputValue);
		}

		TChunrealValueGetterOperator(const FOperatorSettings& InSettings, const FChuckInstanceReadRef& InChuckInstance, FStringReadRef& InParamName, TDataWriteReference<ValueType>& InOutputValue)
			: ChuckInstance(InChuckInstance)
			, ParamName(InParamName)
			,OutputValue(InOutputValue)
		{
			//OutputValue = MakeUnique<TDataWriteReference<ValueType>>(InSettings, METASOUND_GET_PARAM_NAME(OutputValue));
		}

		virtual ~TChunrealValueGetterOperator() = default;

		virtual void BindInputs(FInputVertexInterfaceData& InOutVertexData) override
		{
			using namespace ValueVertexNames;
			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(ChuckInstance), ChuckInstance);
			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(ParamName), ParamName);
		}

		virtual void BindOutputs(FOutputVertexInterfaceData& InOutVertexData) override
		{
			using namespace ValueVertexNames;
			InOutVertexData.BindWriteVertex(METASOUND_GET_PARAM_NAME(OutputValue), OutputValue);
		}

		
		void Execute()
		{
			//if chuck instance is set and has a valid ChukRef, set output value to the value of the parameter
			if (ChuckInstance->IsInitialized() && ChuckInstance->GetProxy()->ChuckInstance->ChuckVm != nullptr)
			{
				*OutputValue = TChunrealValue<ValueType>::GetValueFromChuck(ChuckInstance->GetProxy()->ChuckInstance->ChuckVm, *ParamName);
			}
			else
			{
				return;
			}
			

		}

		void Reset(const IOperator::FResetParams& InParams)
		{
			//OutputValue->Reset();
		}

	protected:
			TDataReadReference<FChuckInstance> ChuckInstance;
			TDataReadReference<FString> ParamName;
			TDataWriteReference<ValueType> OutputValue;

			ChucK* ChuckPointer = nullptr;


	};


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

				return ChuckValueNodePrivate::CreateSetterNodeClassMetadata(DataTypeName, OperatorName, NodeDisplayName, NodeDescription, NodeInterface);
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


			TDataReadReference<ValueType> TargetValue = InputData.GetOrCreateDefaultDataReadReference<ValueType>(METASOUND_GET_PARAM_NAME(InputTargetValue), InParams.OperatorSettings);

			return MakeUnique<TChunrealValueOperator<ValueType>>(InParams.OperatorSettings, ChuckInstance, ParamName, TargetValue);
		}


		TChunrealValueOperator(const FOperatorSettings& InSettings, const FChuckInstanceReadRef& InChuckInstance, FStringReadRef& InParamName,  const TDataReadReference<ValueType>& InTargetValue)
			: ChuckInstance(InChuckInstance)
			, ParamName(InParamName)
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

			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InputTargetValue), TargetValue);
		}

		virtual void BindOutputs(FOutputVertexInterfaceData& InOutVertexData) override
		{
			using namespace ValueVertexNames;

		}



		void Execute()
		{
			//if no chuck proxy and initialized with chuckvm, return cause otherwise we'll crash 
			if (!ChuckInstance->IsInitialized() || ChuckInstance->GetProxy()->ChuckInstance->ChuckVm == nullptr)
			{
				return;
			}
			
			if (!bInitialValueSet)
			{
				CurrentValue = *TargetValue;
				TChunrealValue<ValueType>::SetValueToChuck(ChuckInstance->GetProxy()->ChuckInstance->ChuckVm, *ParamName, *TargetValue);
				ChuckInstance->GetProxy()->ChuckInstance->ChuckVm->globals()->broadcastGlobalEvent(TCHAR_TO_ANSI(TEXT("paramUpdate")));
				bInitialValueSet = true;
			}

			if (CurrentValue != *TargetValue)
			{
				CurrentValue = *TargetValue;
				TChunrealValue<ValueType>::SetValueToChuck(ChuckInstance->GetProxy()->ChuckInstance->ChuckVm, *ParamName, *TargetValue);
				ChuckInstance->GetProxy()->ChuckInstance->ChuckVm->globals()->broadcastGlobalEvent(TCHAR_TO_ANSI(TEXT("paramUpdate")));
			}


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
		TDataReadReference<ValueType> TargetValue;
		ValueType CurrentValue;
		bool bInitialValueSet = false;

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


	template<typename ValueType>
	class CHUNREAL_API TChunrealValueGetterNode : public FNodeFacade
	{

	public:
		/**
		 * Constructor used by the Metasound Frontend.
		 */
		TChunrealValueGetterNode(const FNodeInitData& InInitData)
			: FNodeFacade(InInitData.InstanceName, InInitData.InstanceID, TFacadeOperatorClass<TChunrealValueGetterOperator<ValueType>>())
		{}

		virtual ~TChunrealValueGetterNode() = default;
	};


} // namespace Metasound

#undef LOCTEXT_NAMESPACE
