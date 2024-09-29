// Copyright Epic Games, Inc. All Rights Reserved.

#include "CodeProjectEditorToolbar.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "CodeProjectEditorCommands.h"
#include "LevelEditorActions.h"
#include "SourceCodeNavigation.h"
#include "UObject/UObjectIterator.h"
#include "MetasoundSource.h"
#include <ChuckInstance.h>


void FCodeProjectEditorToolbar::AddEditorToolbar(TSharedPtr<FExtender> Extender)
{
	check(CodeProjectEditor.IsValid());
	TSharedPtr<FCodeProjectEditor> CodeProjectEditorPtr = CodeProjectEditor.Pin();

	Extender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		CodeProjectEditorPtr->GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateSP( this, &FCodeProjectEditorToolbar::FillEditorToolbar ) );
}

void FCodeProjectEditorToolbar::FillEditorToolbar(FToolBarBuilder& ToolbarBuilder)
{
	TSharedPtr<FCodeProjectEditor> CodeProjectEditorPtr = CodeProjectEditor.Pin();

	ToolbarBuilder.BeginSection(TEXT("FileManagement"));
	{
		ToolbarBuilder.AddToolBarButton(FCodeProjectEditorCommands::Get().Save);
		ToolbarBuilder.AddToolBarButton(FCodeProjectEditorCommands::Get().SaveAll);
	}
	ToolbarBuilder.EndSection();

	// Only show the compile options on machines with the solution (assuming they can build it)
	if ( FSourceCodeNavigation::IsCompilerAvailable() )
	{
		ToolbarBuilder.BeginSection(TEXT("Build"));
		{
			struct Local
			{
				static void ExecuteCompile(TSharedPtr<FCodeProjectEditor> InCodeProjectEditorPtr)
				{
					if(InCodeProjectEditorPtr->SaveAll())
					{
						FLevelEditorActionCallbacks::RecompileGameCode_Clicked();
					}
				}

				static void PrintAllChuckAsetNames(TSharedPtr<FCodeProjectEditor> InCodeProjectEditorPtr)
				{
					//get all UChuckProcessors
					TArray<UChuckProcessor*> ChuckProcessors;
					for (TObjectIterator<UChuckProcessor> Itr; Itr; ++Itr)
					{
						ChuckProcessors.Add(*Itr);
					}

					//print their names
					for (UChuckProcessor* ChuckProcessor : ChuckProcessors)
					{
						bool bIsAsset = ChuckProcessor->IsAsset();
						bool bIsInTransientPackage = ChuckProcessor->GetPackage() == GetTransientPackage();
						FString FormattedOutput = FString::Printf(TEXT("ChuckProcessor: %s. Is Asset: %s, Is Transient %s"), *ChuckProcessor->GetName(), ChuckProcessor->IsAsset() ? TEXT("true") : TEXT("false"), bIsInTransientPackage ? TEXT("true") : TEXT("false"));
						
						UE_LOG(LogTemp, Warning, TEXT("ChuckProcessor: %s"), *FormattedOutput);
					}

				}
			};

			// Since we can always add new code to the project, only hide these buttons if we haven't done so yet
			ToolbarBuilder.AddToolBarButton(
				FUIAction(
					FExecuteAction::CreateStatic(&Local::ExecuteCompile, CodeProjectEditorPtr),
					FCanExecuteAction::CreateStatic(&FLevelEditorActionCallbacks::Recompile_CanExecute),
					FIsActionChecked(),
					FIsActionButtonVisible::CreateStatic(FLevelEditorActionCallbacks::CanShowSourceCodeActions)),
				NAME_None,
				NSLOCTEXT( "LevelEditorToolBar", "CompileMenuButton", "Compile" ),
				NSLOCTEXT( "LevelEditorActions", "RecompileGameCode_ToolTip", "Recompiles and reloads C++ code for game systems on the fly" ),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Recompile")
				);

			//add a button that finds all UChuckProcessors and prints their names
			ToolbarBuilder.AddToolBarButton(
				FUIAction(
					FExecuteAction::CreateStatic(&Local::PrintAllChuckAsetNames, CodeProjectEditorPtr),
					//FCanExecuteAction::CreateStatic(&FLevelEditorActionCallbacks::Recompile_CanExecute),
					FIsActionChecked(),
					FIsActionButtonVisible::CreateStatic(FLevelEditorActionCallbacks::CanShowSourceCodeActions)),
				NAME_None,
				NSLOCTEXT("LevelEditorToolBar", "CompileMenuButton", "Print Chuck Processors"),
				NSLOCTEXT("LevelEditorActions", "RecompileGameCode_ToolTip", "Prints the names of all UChuckProcessors"),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Recompile")
			);
		}
		ToolbarBuilder.EndSection();
	}
}
