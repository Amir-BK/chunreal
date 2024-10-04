// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChunrealEditor.h"

#include "PropertyEditorModule.h"

#include "UObject/UObjectArray.h"

#include <filesystem>
#include <iostream>


#define LOCTEXT_NAMESPACE "FChunrealEditor"

static const FName CodeEditorTabName(TEXT("ChucKEditor"));

void FChunrealEditor::StartupModule()
{
	ChuckInstanceActionsSharedPtr = MakeShared<FChuckProcessorAssetActions>();
	ChuckInstantiationActionsSharedPtr = MakeShared<FChuckInstantiationAssetActions>();
	FAssetToolsModule::GetModule().Get().RegisterAssetTypeActions(ChuckInstanceActionsSharedPtr.ToSharedRef());
	FAssetToolsModule::GetModule().Get().RegisterAssetTypeActions(ChuckInstantiationActionsSharedPtr.ToSharedRef());

	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout("ChuckProcessor", FOnGetDetailCustomizationInstance::CreateStatic(&FChuckProcessorDetails::MakeInstance));

	//code editor - 
	FCodeEditorStyle::Initialize();

	// Register a tab spawner so that our tab can be automatically restored from layout files
	FGlobalTabmanager::Get()->RegisterTabSpawner(CodeEditorTabName, FOnSpawnTab::CreateStatic(&FChunrealEditor::SpawnCodeEditorTab))
		.SetDisplayName(LOCTEXT("CodeEditorTabTitle", "Edit Source Code"))
		.SetTooltipText(LOCTEXT("CodeEditorTooltipText", "Open the Code Editor tab."))
		.SetIcon(FSlateIcon(FCodeEditorStyle::Get().GetStyleSetName(), "CodeEditor.TabIcon"));

	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FChunrealEditor::OnPostEngineInit);
	

};

void FChunrealEditor::ShutdownModule()
{
	if (!FModuleManager::Get().IsModuleLoaded("AssetTools")) return;
	FAssetToolsModule::GetModule().Get().UnregisterAssetTypeActions(ChuckInstanceActionsSharedPtr.ToSharedRef());
	FAssetToolsModule::GetModule().Get().UnregisterAssetTypeActions(ChuckInstantiationActionsSharedPtr.ToSharedRef());

	//code editor -
			// Unregister the tab spawner
	FGlobalTabmanager::Get()->UnregisterTabSpawner(CodeEditorTabName);

	FCoreDelegates::OnPostEngineInit.RemoveAll(this);
	UToolMenus::UnregisterOwner(this);

	FCodeEditorStyle::Shutdown();
}




#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FChunrealEditor, ChunrealEditor)