// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "CodeProject.h"
#include "CodeProjectFactory.generated.h"

UCLASS()
class CHUNREALEDITOR_API UCodeProjectFactory : public UFactory
{
	GENERATED_BODY()

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	// End of UFactory interface

	UCodeProjectFactory();
};
