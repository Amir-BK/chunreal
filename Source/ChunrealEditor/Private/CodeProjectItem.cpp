// Copyright Epic Games, Inc. All Rights Reserved.

#include "CodeProjectItem.h"
#include "UObject/Package.h"
#include "IDirectoryWatcher.h"
#include "DirectoryWatcherModule.h"
#include "DirectoryScanner.h"
#include "Interfaces/IPluginManager.h"
#include "ChuckInstance.h"
#include "ChunrealEditor.h"
#include "HAL/FileManager.h"

UCodeProjectItem::UCodeProjectItem(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FString BaseDir = IPluginManager::Get().FindPlugin(TEXT("Chunreal"))->GetBaseDir();
	Path = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::Combine(*BaseDir, TEXT("WorkingDirectory")));
}

void UCodeProjectItem::RescanChildren()
{
	if(Path.Len() > 0)
	{
		FDirectoryScanner::AddDirectory(Path, FOnDirectoryScanned::CreateUObject(this, &UCodeProjectItem::HandleDirectoryScanned));
	}
}

void UCodeProjectItem::HandleDirectoryScanned(const FString& InPathName, ECodeProjectItemType::Type InType)
{
	// check for a child that already exists
	bool bCreateNew = true;
	for(const auto& Child : Children)
	{
		if(Child->Type == InType && Child->Path == InPathName)
		{
			bCreateNew = false;
			break;
		}
	}

	// create children now & kick off their scan
	if(bCreateNew)
	{
		UCodeProjectItem* NewItem = NewObject<UCodeProjectItem>(GetOutermost(), UCodeProjectItem::StaticClass());
		NewItem->Type = InType;
		NewItem->Path = InPathName;
		NewItem->Name = FPaths::GetCleanFilename(InPathName);
		if(InType != ECodeProjectItemType::Folder)
		{
			NewItem->Extension = FPaths::GetExtension(InPathName);
		}

		Children.Add(NewItem);

		Children.Sort(
			[](const UCodeProjectItem& ItemA, const UCodeProjectItem& ItemB) -> bool
			{
				if(ItemA.Type != ItemB.Type)
				{
					return ItemA.Type < ItemB.Type;
				}

				return ItemA.Name.Compare(ItemB.Name) < 0;
			}
		);

		if(InType == ECodeProjectItemType::Folder)
		{
			// kick off another scan for subdirectories
			FDirectoryScanner::AddDirectory(InPathName, FOnDirectoryScanned::CreateUObject(NewItem, &UCodeProjectItem::HandleDirectoryScanned));

			// @TODO: now register for any changes to this directory if needed
			FDirectoryWatcherModule& DirectoryWatcherModule = FModuleManager::Get().LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
			//DirectoryWatcherModule.Get()->RegisterDirectoryChangedCallback_Handle(InPathName, IDirectoryWatcher::FDirectoryChanged::CreateUObject(NewItem, &UCodeProjectItem::HandleDirectoryChanged), OnDirectoryChangedHandle);
		}
	}
}

void UCodeProjectItem::HandleDirectoryChanged(const TArray<FFileChangeData>& FileChanges)
{
	// @TODO: dynamical update directory watchers so we can update the view in real-time
	TArray<FString> ScannedFiles; //I don't understand why but this can contain two modifications for a file... we don't want that.
	for(const auto& Change : FileChanges)
	{
		UE_LOG(LogTemp, Log, TEXT("File change: %s"), *Change.Filename);
		if (ScannedFiles.Contains(Change.Filename))
		{
			continue;
		}

		ScannedFiles.Add(Change.Filename);

		switch(Change.Action)
		{
		default:
		case FFileChangeData::FCA_Unknown:
			break;
		case FFileChangeData::FCA_Added:
			{
				//is file a chuck file?
			if (Change.Filename.EndsWith(TEXT(".ck")))
			{
				FChunrealEditor::ScanWorkingDirectoryAndUpdateRuntimeAssets();
			}

			}
			break;
		case FFileChangeData::FCA_Modified:
			{
			if (Change.Filename.EndsWith(TEXT(".ck")))
			{
				auto* ChuckProcessor = FChunrealEditor::GetProcessorProxyForChuck(Change.Filename);
				if (ChuckProcessor)
				{
					//FGuid OldGuid = ChuckProcessor->ChuckGuid;
					//ChuckProcessor->ChuckGuid = FGuid::NewGuid();
					ChuckProcessor->OnChuckNeedsRecompile.Broadcast();
				//	UE_LOG(LogTemp, Log, TEXT("Chuck file: %s, has been modified. Old Guid: %s, New Guid: %s"), *Change.Filename, *OldGuid.ToString(), *ChuckProcessor->ChuckGuid.ToString());
				}
			}
			}
			break;
		case FFileChangeData::FCA_Removed:
			{
			if (Change.Filename.EndsWith(TEXT(".ck")))
			{
				auto* ChuckProcessor = FChunrealEditor::GetProcessorProxyForChuck(Change.Filename);
				if (ChuckProcessor)
				{
					//ChuckProcessor->ChuckGuid = FGuid::NewGuid();
					ChuckProcessor->SourcePath = TEXT("");
				}
			}
			}
			break;
		}
	}
}
