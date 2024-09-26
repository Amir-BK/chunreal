// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ChunrealAssetClasses.h"
#include "Modules/ModuleManager.h"


class FChunrealEditor final : public IModuleInterface
{
public:



	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;


private:

	TSharedPtr<FChuckInstanceAssetActions> ChuckInstanceActionsSharedPtr;
	//TSharedPtr<FChunrealAssetStyles> ChunrealStylesSharedPtr;

};
