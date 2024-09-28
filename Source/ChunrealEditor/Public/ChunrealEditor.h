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


class FChunrealEditor final : public IModuleInterface
{
public:



	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual void OnPostEngineInit()
	{
		if (UToolMenus::IsToolMenuUIEnabled())
		{
			UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
			FToolMenuSection& Section = Menu->FindOrAddSection("FileProject");

			FToolMenuOwnerScoped OwnerScoped(this);
			{
				FToolMenuEntry& MenuEntry = Section.AddMenuEntry(
					"EditSourceCode",
					INVTEXT("Edit ChucK Source Code"),
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
