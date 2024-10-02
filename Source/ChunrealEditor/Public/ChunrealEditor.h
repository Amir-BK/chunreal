// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ChunrealAssetClasses.h"
#include "Misc/CoreDelegates.h"
#include "ToolMenus.h"
#include "CodeEditorStyle.h"
#include "CodeProject.h"
#include "CodeProjectEditor.h"
#include "ToolMenu.h"
#include "ToolMenuEntry.h"
#include "ToolMenuSection.h"
#include "Modules/ModuleManager.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManagerGeneric.h"
#include "Engine/AssetManager.h"
#include "ChunrealAssetClasses.h"
#include "Interfaces/IPluginManager.h"
#include "ObjectTools.h"
#include "IAssetTools.h"


class FChunrealEditor final : public IModuleInterface
{
public:



	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static UChuckProcessor* GetProcessorProxyForChuck(const FString& InChuckPath)
	{
		const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		TArray<FAssetData> AssetData;
		AssetRegistryModule.Get().GetAssetsByClass(UChuckProcessor::StaticClass()->GetClassPathName(), AssetData, true);
		FString ChuckName = FPaths::GetBaseFilename(InChuckPath);
		ChuckName.RemoveSpacesInline();

		FString ChuckAssetPath = TEXT("/Chunreal/Chunreal/RuntimeChucks/") + ChuckName + TEXT(".") + ChuckName;
		FName ChuckAssetName = FName(*ChuckAssetPath);
		int ExistingAssetIndex = AssetData.IndexOfByPredicate([&ChuckAssetName](const FAssetData& AssetData)
			{
				return AssetData.ObjectPath.IsEqual(ChuckAssetName);
			});

		if (ExistingAssetIndex != INDEX_NONE)
		{
			return Cast<UChuckProcessor>(AssetData[ExistingAssetIndex].GetAsset());
		}
		else
		{
			return nullptr;
		}
	}

	static void ScanWorkingDirectoryAndUpdateRuntimeAssets()
	{
		// so, in theory this will only run when we have an editor, but we don't want to block completly the option to add chucks without an editor, we'll see
		// to avoid messing with race conditions for now (in the future probably a delegate from the runtime module), just assume we do nothing in the runtime
		FString BaseDir = IPluginManager::Get().FindPlugin(TEXT("Chunreal"))->GetBaseDir();
		FString WorkingDir = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::Combine(*BaseDir, TEXT("WorkingDirectory")));

		TArray<FString> ChuckFiles;
		FFileManagerGeneric::Get().FindFiles(ChuckFiles, *WorkingDir, TEXT("ck"));
		const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		auto& AssetTools = IAssetTools::Get();
		auto* Factory = NewObject<UChuckInstanceFactory>();

		TArray<FAssetData> AssetData;
		AssetRegistryModule.Get().GetAssetsByClass(UChuckProcessor::StaticClass()->GetClassPathName(), AssetData, true);

		

		//for out test, construct a transient UChuckProcessor for each chuck file
		for (const FString& ChuckFile : ChuckFiles)
		{
			//we need the file name without the extensions
			FString ChuckName = FPaths::GetBaseFilename(ChuckFile);
			TArray<FString> ResultTokens;
			FFileHelper::LoadFileToStringArrayWithPredicate(ResultTokens, *(WorkingDir + "/" + ChuckFile), [](const FString& Line) { return Line.Contains(TEXT("UCHUCK()")); });
			bool bIsUChuck = ResultTokens.Num() > 0;

			
			//we may already have an asset for this chuck file
			FString ChuckAssetPath = TEXT("/Chunreal/Chunreal/RuntimeChucks/") + ChuckName + TEXT(".") + ChuckName;
			FName ChuckAssetName = FName(*ChuckAssetPath);
			int ExistingAssetIndex = AssetData.IndexOfByPredicate([&ChuckAssetName](const FAssetData& AssetData) 
				{ 
					return AssetData.ObjectPath.IsEqual(ChuckAssetName); 
				
				
				});
			
			UChuckProcessor* ChuckProcessor = nullptr;
			UE_LOG(LogTemp, Log, TEXT("Index : %d"), ExistingAssetIndex);
			if (ExistingAssetIndex != INDEX_NONE)
			{
				ChuckProcessor = Cast<UChuckProcessor>(AssetData[ExistingAssetIndex].GetAsset());
				AssetData.RemoveAtSwap(ExistingAssetIndex);
				UE_LOG(LogTemp, Log, TEXT("Found Chuck file: %s, already exists as asset."), *ChuckFile);
	
			}
			else {

				UObject* ChuckNewObject = AssetTools.CreateAsset(ChuckName, TEXT("/chunreal/chunreal/RuntimeChucks"), UChuckProcessor::StaticClass(), Factory);
				ChuckProcessor = Cast<UChuckProcessor>(ChuckNewObject);
				ChuckProcessor->bIsAutoManaged = true;
				ChuckProcessor->SourcePath = WorkingDir + "/" + ChuckFile;
				ChuckProcessor->ChuckGuid = FGuid::NewGuid();
				AssetRegistryModule.Get().AssetCreated(ChuckProcessor);
			}

		}

		//ObjectTools::D

		//delete any remaining assets, with dialog?
		if (AssetData.Num() > 0)
		{
			ObjectTools::DeleteAssets(AssetData, true);
		}
		



	}

	
	

	virtual void OnPostEngineInit()
	{
		if (UToolMenus::IsToolMenuUIEnabled())
		{
			UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("MainFrame.MainMenu.Tools");
			FToolMenuSection& Section = Menu->FindOrAddSection("Programming");

			FToolMenuOwnerScoped OwnerScoped(this);
			{
				FToolMenuEntry& MenuEntry = Section.AddMenuEntry(
					"EditSourceCode",
					INVTEXT("Edit ChucK Project Code"),
					INVTEXT("Open the ChucK Editor tab."),
					FSlateIcon(FCodeEditorStyle::Get().GetStyleSetName(), "CodeEditor.TabIcon"),
					FUIAction
					(
						FExecuteAction::CreateStatic(&FChunrealEditor::OpenCodeEditor)
					)
				);
				MenuEntry.InsertPosition = FToolMenuInsert(NAME_None, EToolMenuInsertType::First);
			}
		}

		ScanWorkingDirectoryAndUpdateRuntimeAssets();
	}


private:

	TSharedPtr<FChuckInstanceAssetActions> ChuckInstanceActionsSharedPtr;
	//TSharedPtr<FChunrealAssetStyles> ChunrealStylesSharedPtr;

	static TSharedRef<SDockTab> SpawnCodeEditorTab(const FSpawnTabArgs& TabArgs)
	{
		TSharedRef<FCodeProjectEditor> NewCodeProjectEditor(new FCodeProjectEditor());
		NewCodeProjectEditor->InitCodeEditor(EToolkitMode::Standalone, TSharedPtr<class IToolkitHost>(), GetMutableDefault<UCodeProject>());

		return FGlobalTabmanager::Get()->GetMajorTabForTabManager(NewCodeProjectEditor->GetTabManager().ToSharedRef()).ToSharedRef();
	}

	static void OpenCodeEditor()
	{
		SpawnCodeEditorTab(FSpawnTabArgs(TSharedPtr<SWindow>(), FTabId()));
	}

};
