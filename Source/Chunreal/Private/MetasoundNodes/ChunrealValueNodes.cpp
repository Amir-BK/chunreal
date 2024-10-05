#include "MetasoundNodes/ChunrealValueNodes.h"

#define LOCTEXT_NAMESPACE "ChunrealMetasound_ChunrealValueNode"

namespace ChunrealMetasound
{
	namespace ChuckValueNodePrivate
	{
		FNodeClassMetadata CreateSetterNodeClassMetadata(const FName& InDataTypeName, const FName& InOperatorName, const FText& InDisplayName, const FText& InDescription, const FVertexInterface& InDefaultInterface)
		{
			FNodeClassMetadata Metadata
			{
				FNodeClassName{ "ChuckValue", InOperatorName, InDataTypeName },
				1, // Major Version
				0, // Minor Version
				InDisplayName,
				InDescription,
				PluginAuthor,
				PluginNodeMissingPrompt,
				InDefaultInterface,
				{ INVTEXT("Chuck Value") },
				{ },
				FNodeDisplayStyle{}
			};

			return Metadata;
		}
		FNodeClassMetadata CreateGetterNodeClassMetadata(const FName& InDataTypeName, const FName& InOperatorName, const FText& InDisplayName, const FText& InDescription, const FVertexInterface& InDefaultInterface)
		{
			FNodeClassMetadata Metadata
			{
				FNodeClassName{ "ChuckValueGetter", InOperatorName, InDataTypeName },
				1, // Major Version
				0, // Minor Version
				InDisplayName,
				InDescription,
				PluginAuthor,
				PluginNodeMissingPrompt,
				InDefaultInterface,
				{ INVTEXT("Chuck Value Getter") },
				{ },
				FNodeDisplayStyle{}
			};

			return Metadata;
		}
	}

	

	//value setters
	using FValueNodeInt32 = TChunrealValueNode<int32>;
	METASOUND_REGISTER_NODE(FValueNodeInt32)

		using FValueNodeFloat = TChunrealValueNode<float>;
	METASOUND_REGISTER_NODE(FValueNodeFloat)

		//using FValueNodeBool = TValueNode<bool>;
	//METASOUND_REGISTER_NODE(FValueNodeBool)

		using FValueNodeString = TChunrealValueNode<FString>;
	METASOUND_REGISTER_NODE(FValueNodeString)

		using FValueFloatArray = TChunrealValueNode<TArray<float>>;
	METASOUND_REGISTER_NODE(FValueFloatArray)

		using FValueIntArray = TChunrealValueNode<TArray<int32>>;
	METASOUND_REGISTER_NODE(FValueIntArray)

	//	using FValueStringArray = TChunrealValueNode<TArray<FString>>;
	//METASOUND_REGISTER_NODE(FValueStringArray)

		//value getters
		using TChunrealValueInt32 = TChunrealValueGetterNode<int32>;
	METASOUND_REGISTER_NODE(TChunrealValueInt32)
		using TChunrealValueFloat = TChunrealValueGetterNode<float>;
	METASOUND_REGISTER_NODE(TChunrealValueFloat)

		using TChunrealValueString = TChunrealValueGetterNode<FString>;
	METASOUND_REGISTER_NODE(TChunrealValueString)
		using TChunrealValueFloatArray = TChunrealValueGetterNode<TArray<float>>;
	METASOUND_REGISTER_NODE(TChunrealValueFloatArray)
		using TChunrealValueIntArray = TChunrealValueGetterNode<TArray<int32>>;
	METASOUND_REGISTER_NODE(TChunrealValueIntArray)
	//	using TChunrealValueStringArray = TChunrealValueGetterNode<TArray<FString>>;

}
