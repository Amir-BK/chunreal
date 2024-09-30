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
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "ScopedTransaction.h"
#include "ChunrealAssetClasses.generated.h"

class FChuckInstanceAssetActions : public FAssetTypeActions_Base
{
public:

	UClass* GetSupportedClass() const override
	{
		return UChuckProcessor::StaticClass();
	}
	FText GetName() const override
	{
		return INVTEXT("ChucK Processor");
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
		
		const static FSlateRoundedBoxBrush RecessedBrush(FStyleColors::Recessed, CoreStyleConstants::InputFocusRadius);
		const static FEditableTextBoxStyle InfoWidgetStyle =
			FEditableTextBoxStyle(FAppStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>("NormalEditableTextBox"))
			.SetBackgroundImageNormal(RecessedBrush)
			.SetBackgroundImageHovered(RecessedBrush)
			.SetBackgroundImageFocused(RecessedBrush)
			.SetBackgroundImageReadOnly(RecessedBrush);
		
		TArray<TWeakObjectPtr<UObject>> Outers;
		DetailBuilder.GetObjectsBeingCustomized(Outers);
		if (Outers.Num() == 0) return;
		ChuckInstance = Cast<UChuckProcessor>(Outers[0].Get());

		//add category "ChucK" and add a MultiLineEditableText to is with the ChucK Marshaller
		IDetailCategoryBuilder& ChucKCategory = DetailBuilder.EditCategory("Chuck");
		ChucKCategory.AddCustomRow(FText::FromString("ChucK Code"))
			.NameContent()
			.MaxDesiredWidth(150)
			[
				SNew(STextBlock)
					.Text(FText::FromString("ChucK Code"))
			]
			.ValueContent()
			.MinDesiredWidth(800)
			.MaxDesiredWidth(600)
			[
				SNew(SBorder)
					.BorderBackgroundColor(FLinearColor::Black)

					[
						SNew(SMultiLineEditableTextBox)
							.Text_Lambda([this]() { return FText::FromString(*ChuckInstance->Code); })
							.OnTextCommitted_Lambda([this](const FText& InText, ETextCommit::Type CommitType)
								{
									FScopedTransaction Transaction(INVTEXT("Update Chuck Code"));
									ChuckInstance->Code = InText.ToString();
									ChuckInstance->MarkPackageDirty();
								})
							.Marshaller(FChucKSyntaxHighlighterMarshaller::Create())
							.Style(&InfoWidgetStyle)
							.AlwaysShowScrollbars(true)
							.IsReadOnly(ChuckInstance->bIsAutoManaged)
					]

			];

	}

	static TSharedRef<IDetailCustomization> MakeInstance() { return MakeShareable(new FChuckInstanceDetails()); }

private:
	UChuckProcessor* ChuckInstance = nullptr;

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
		//print object flags

		
		UChuckProcessor* NewInstance = NewObject<UChuckProcessor>(InParent, InClass, InName, Flags);
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
		SupportedClass = UChuckProcessor::StaticClass();
	}
};
