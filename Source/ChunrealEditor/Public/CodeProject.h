// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CodeProjectItem.h"
#include "CodeProject.generated.h"

UCLASS(Blueprintable)
class UCodeProject : public UCodeProjectItem
{
	GENERATED_UCLASS_BODY()

	// @TODO: This class should probably be mostly config/settings stuff, with a details panel allowing editing somewhere
};
