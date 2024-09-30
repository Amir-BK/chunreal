// Copyright Epic Games, Inc. All Rights Reserved.

#include "CodeProject.h"
//#include "Chunreal.h"
#include "Interfaces/IPluginManager.h"
#include "DirectoryWatcherModule.h"
#include "IDirectoryWatcher.h"
#include "DirectoryScanner.h"
#include "HAL/FileManager.h"
	#include "Modules/ModuleManager.h"
#include "Misc/Paths.h"


UCodeProject::UCodeProject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//FChunrealModule ChunrealModule = FModuleManager::Get().GetModuleChecked<FChunrealModule>("Chunreal");
	//Path = ChunrealModule.workingDirectory;
	FString BaseDir = IPluginManager::Get().FindPlugin(TEXT("Chunreal"))->GetBaseDir();
	Path = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::Combine(*BaseDir, TEXT("WorkingDirectory")));


	// @TODO: now register for any changes to this directory if needed
	FDirectoryWatcherModule& DirectoryWatcherModule = FModuleManager::Get().LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
	DirectoryWatcherModule.Get()->RegisterDirectoryChangedCallback_Handle(Path, IDirectoryWatcher::FDirectoryChanged::CreateUObject(this, &UCodeProjectItem::HandleDirectoryChanged), OnDirectoryChangedHandle);
}
