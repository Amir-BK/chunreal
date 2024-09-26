// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "AssetTypeActions_Base.h"
#include "Widgets/SWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Images/SImage.h"
#include "AssetRegistry/AssetData.h"
#include "ChuckInstance.h"
#include "ChunrealAssetClasses.generated.h"

class FChuckInstanceAssetActions : public FAssetTypeActions_Base
{
public:

	UClass* GetSupportedClass() const override
	{
		return UChuckInstance::StaticClass();
	}
	FText GetName() const override
	{
		return INVTEXT("ChucK Instance");
	}
	FColor GetTypeColor() const override
	{
		return FColor::Purple;
	}
	uint32 GetCategories() override
	{
		return EAssetTypeCategories::Sounds;
	}

};

/**
 *
 */
UCLASS()
class CHUNREALEDITOR_API UChuckInstanceFactory : public UFactory
{
	GENERATED_BODY()  

public:
	//~ UFactory Interface
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override
	{
		UChuckInstance* NewInstance = NewObject<UChuckInstance>(InParent, InClass, InName, Flags);
		return NewInstance;
	}

	virtual bool ShouldShowInNewMenu() const override
	{
		return true;
	}

	UChuckInstanceFactory()
	{
		bCreateNew = true;
		bEditAfterNew = true;
		SupportedClass = UChuckInstance::StaticClass();
	}
};
