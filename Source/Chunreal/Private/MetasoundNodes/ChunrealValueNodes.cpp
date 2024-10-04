#include "MetasoundNodes/ChunrealValueNodes.h"

#define LOCTEXT_NAMESPACE "ChunrealMetasound_ChunrealValueNode"

namespace ChunrealMetasound
{
	namespace MetasoundValueNodePrivate
	{
		FNodeClassMetadata CreateNodeClassMetadata(const FName& InDataTypeName, const FName& InOperatorName, const FText& InDisplayName, const FText& InDescription, const FVertexInterface& InDefaultInterface)
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
	}


	using FValueNodeInt32 = TChunrealValueNode<int32>;
	METASOUND_REGISTER_NODE(FValueNodeInt32)

		using FValueNodeFloat = TChunrealValueNode<float>;
	METASOUND_REGISTER_NODE(FValueNodeFloat)

		//using FValueNodeBool = TValueNode<bool>;
	//METASOUND_REGISTER_NODE(FValueNodeBool)

		using FValueNodeString = TChunrealValueNode<FString>;
	METASOUND_REGISTER_NODE(FValueNodeString)
}
