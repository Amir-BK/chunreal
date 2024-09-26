// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChunrealEditor.h"

#include "PropertyEditorModule.h"

#include "UObject/UObjectArray.h"


#define LOCTEXT_NAMESPACE "FChunrealEditor"

void FChunrealEditor::StartupModule()
{
	ChuckInstanceActionsSharedPtr = MakeShared<FChuckInstanceAssetActions>();
	FAssetToolsModule::GetModule().Get().RegisterAssetTypeActions(ChuckInstanceActionsSharedPtr.ToSharedRef());
	

};

void FChunrealEditor::ShutdownModule()
{
	if (!FModuleManager::Get().IsModuleLoaded("AssetTools")) return;
	FAssetToolsModule::GetModule().Get().UnregisterAssetTypeActions(ChuckInstanceActionsSharedPtr.ToSharedRef());
}



#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FChunrealEditor, ChunrealEditor)