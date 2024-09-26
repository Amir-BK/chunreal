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
#include "IDetailCustomization.h"
#include "DetailLayoutBuilder.h"

#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "ChucKSyntaxHighlighter.h"
#include "Widgets/Text/SMultiLineEditableText.h"
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

//detail customization
class FChuckInstanceDetails : public IDetailCustomization
{
public:
	// This function will be called when the properties are being customized
	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override
	{
		TArray<TWeakObjectPtr<UObject>> Outers;
		DetailBuilder.GetObjectsBeingCustomized(Outers);
		if (Outers.Num() == 0) return;
		ChuckInstance = Cast<UChuckInstance>(Outers[0].Get());

		//add category "ChucK" and add a MultiLineEditableText to is with the ChucK Marshaller
		IDetailCategoryBuilder& ChucKCategory = DetailBuilder.EditCategory("Chuck");
		ChucKCategory.AddCustomRow(FText::FromString("ChucK Code"))
			.NameContent()
			[
				SNew(STextBlock)
					.Text(FText::FromString("ChucK Code"))
			]
			.ValueContent()
			[
				SNew(SBorder)
					.BorderBackgroundColor(FLinearColor::Black)
					.ColorAndOpacity(FLinearColor::Black)

					[
						SNew(SMultiLineEditableText)
							.Text(FText::FromString(ChuckInstance->Code))

							.OnTextChanged_Lambda([this](const FText& InText)
								{
									ChuckInstance->Code = *InText.ToString();
								})
	
							.Marshaller(FChucKSyntaxHighlighterMarshaller::Create())
					]

			];

	}

	static TSharedRef<IDetailCustomization> MakeInstance() { return MakeShareable(new FChuckInstanceDetails()); }

private:
	UChuckInstance* ChuckInstance = nullptr;

};

	// This function will create a new instance of this class as a shared

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
