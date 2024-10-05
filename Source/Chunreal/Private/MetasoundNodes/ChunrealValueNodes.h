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
	template<typename ValueType>
	struct TChunrealValue;


	template<>
	struct TChunrealValue<int32>
	{
		static int32 GetValueFromChuck(ChucK* ChuckRef, const FString& ParamName)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Getting int value from Chuck"));
			
			return ChuckRef->globals()->get_global_int_value(TCHAR_TO_ANSI(*ParamName));
		}

		static void SetValueToChuck(ChucK* ChuckRef, const FString& ParamName, int32 Value)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Setting int value to Chuck"));
			ChuckRef->globals()->setGlobalInt(TCHAR_TO_ANSI(*ParamName), Value);
		}
	};

	template<>
	struct TChunrealValue<float>
	{
		static float GetValueFromChuck(ChucK* ChuckRef, const FString& ParamName)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Getting float value from Chuck"));
			return ChuckRef->globals()->get_global_float_value(TCHAR_TO_ANSI(*ParamName));
		}

		static void SetValueToChuck(ChucK* ChuckRef, const FString& ParamName, float Value)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Setting float value to Chuck"));
			ChuckRef->globals()->setGlobalFloat(TCHAR_TO_ANSI(*ParamName), Value);
		}
	};

	template<>
	struct TChunrealValue<FString>
	{
		static FString GetValueFromChuck(ChucK* ChuckRef, const FString& ParamName)
		{
			const char* string = ChuckRef->globals()->get_global_string(TCHAR_TO_ANSI(*ParamName))->str().c_str();
			return ANSI_TO_TCHAR(string);

		}

		static void SetValueToChuck(ChucK* ChuckRef, const FString& ParamName, const FString& Value)
		{
			ChuckRef->globals()->setGlobalString(TCHAR_TO_ANSI(*ParamName), TCHAR_TO_ANSI(*Value));
		}
	};

	//array types
	template<>
	struct TChunrealValue<TArray<int32>>
	{
		static TArray<int32> GetValueFromChuck(ChucK* ChuckRef, const FString& ParamName)
		{
			t_CKUINT result = 0;
			TArray<int32> Array;
			Chuck_Object* CkArray = ChuckRef->globals()->get_global_array(TCHAR_TO_ANSI(*ParamName));
			if (CkArray != NULL)
			{
				Chuck_ArrayInt* intArray = (Chuck_ArrayInt*)CkArray;
				for (t_CKUINT i = 0; i < intArray->size(); i++)
				{
					
				intArray->get(i, &result);
				Array.Add(result);
				}
			}

			return Array;
			

		}

		static void SetValueToChuck(ChucK* ChuckRef, const FString& ParamName, const TArray<int32>& Value)
		{
				ChuckRef->globals()->setGlobalIntArray(TCHAR_TO_ANSI(*ParamName), (t_CKINT*) Value.GetData(), Value.Num());
		}
	};

	template<>
	struct TChunrealValue<TArray<float>>
	{
		static TArray<float> GetValueFromChuck(ChucK* ChuckRef, const FString& ParamName)
		{
			t_CKFLOAT result = 0;
			TArray<float> Array;
			Chuck_Object* CkArray = ChuckRef->globals()->get_global_array(TCHAR_TO_ANSI(*ParamName));
			if (CkArray != NULL)
			{
				Chuck_ArrayFloat* floatArray = (Chuck_ArrayFloat*)CkArray;
				for (t_CKUINT i = 0; i < floatArray->size(); i++)
				{
					floatArray->get(i, &result);
					Array.Add(result);
				}
			}

			return Array;
		}

		static void SetValueToChuck(ChucK* ChuckRef, const FString& ParamName, const TArray<float>& Value)
		{

			ChuckRef->globals()->setGlobalFloatArray(TCHAR_TO_ANSI(*ParamName), (double*) Value.GetData(), Value.Num());

		}
	};


	
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
			if (ChuckInstance->IsInitialized() && ChuckInstance->GetProxy()->ChuckInstance->ChuckInstance != nullptr)
			{
				*OutputValue = TChunrealValue<ValueType>::GetValueFromChuck(ChuckInstance->GetProxy()->ChuckInstance->ChuckInstance, *ParamName);
			}
			else
			{
				//UE_LOG(LogTemp, Warning, TEXT("Chuck instance not initialized"));
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



		void Execute()
		{
//			TriggerOnReset->AdvanceBlock();
		//	TriggerOnSet->AdvanceBlock();

			//if (*ResetTrigger)
			{
				//*OutputValue = *InitValue;
			}

			//if (*SetTrigger)
			//{

			//	//*OutputValue = *TargetValue;
			//}

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
				//if chuck instance is set and has a valid ChukRef, set output value to the value of the parameter
					if (ChuckInstance->IsInitialized() && ChuckInstance->GetProxy()->ChuckInstance->ChuckInstance != nullptr)
					{
						TChunrealValue<ValueType>::SetValueToChuck(ChuckInstance->GetProxy()->ChuckInstance->ChuckInstance, *ParamName, *TargetValue);
						UE_LOG(LogTemp, Warning, TEXT("Setting value to Chuck"));
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("Chuck instance not initialized"));
						return;
					}
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
