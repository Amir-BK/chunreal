// Copyright Epic Games, Inc. All Rights Reserved.

#include "CodeProject.h"
//#include "Chunreal.h"
#include "Interfaces/IPluginManager.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"


UCodeProject::UCodeProject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//FChunrealModule ChunrealModule = FModuleManager::Get().GetModuleChecked<FChunrealModule>("Chunreal");
	//Path = ChunrealModule.workingDirectory;
	FString BaseDir = IPluginManager::Get().FindPlugin(TEXT("Chunreal"))->GetBaseDir();
	Path = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::Combine(*BaseDir, TEXT("WorkingDirectory")));
}
