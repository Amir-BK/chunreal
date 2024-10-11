// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/Text/SMultiLineEditableText.h"
#include "ChucKSyntaxHighlighter.h"
#include "SCodeEditableText.h"
#include "ChuckInstance.h"
#include "Components/Button.h"
#include "ChuckCodeEditorWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnChuckCodeChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnChuckWidgetUnfocus);

namespace ChuckCodeEditorWidget
{
	//the default code that will be displayed in the editor
	static const FString DefaultCode = TEXT("<<<Hello World>>>;");
	static const FLinearColor DefaultBackgroundColor = //* dark green */
		FLinearColor(0.0f, 0.2f, 0.0f, 1.0f);
}

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class CHUNREAL_API UChuckCodeEditorWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	TSharedPtr<SBkCodeEditableText> CodeEditor;
	TSharedPtr<SBox> CodeEditorBox;

	//if true, the code object controlled by this widget will be spawned as a copy of the original object, so edits won't affect the original object
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChucK", meta = (ExposeOnSpawn = "true"))
	bool bCreateAsCopy = false;

	// can be useful for examples and the such I guess
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChucK", meta = (ExposeOnSpawn = "true"))
	bool bReadOnly = false; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChucK", meta = (ExposeOnSpawn = "true"))
	FVector2D Size = FVector2D(300.0f, 500.0f);

	UPROPERTY(BlueprintAssignable, Category = "ChucK")
	FOnChuckCodeChanged OnChuckCodeChanged;

	UPROPERTY(BlueprintAssignable, Category = "ChucK")
	FOnChuckWidgetUnfocus OnChuckWidgetUnfocus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChucK")
	TObjectPtr<UChuckCode> ChuckCode;


	//using namespace ChuckCodeEditorWidget;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChucK")
	FLinearColor BackgroundColor = ChuckCodeEditorWidget::DefaultBackgroundColor;

	void NativeConstruct() override
	{
		if (ChuckCode)
		{
			CodeEditor->SetText(FText::FromString(ChuckCode->Code));
		}

	}

	virtual TSharedRef<SWidget> RebuildWidget() override
	{
	

		auto CompileIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Recompile");

		if (CompileButton)
		{
			//compile button icon
			
			CompileButton->SetStyle(
				FButtonStyle().SetNormal(*CompileIcon.GetIcon()));
		}

		auto Hierarchy = 	SNew(SBorder)
			.BorderImage(FAppStyle::Get().GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor(BackgroundColor)
			[
				SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.FillHeight(0.95f)
						[
						SAssignNew(CodeEditorBox, SBox)
							//.MinDesiredWidth(Size.X)
							//.MinDesiredHeight(Size.Y)
							[
								SAssignNew(CodeEditor, SBkCodeEditableText)
									.Text(GetCode())

									.Marshaller(FChucKSyntaxHighlighterMarshaller::Create())
							]
					]
					+ SVerticalBox::Slot()
					.FillHeight(0.05f)
					[
						SNew(SBox)
							.MaxDesiredHeight(15.0f)
							.HeightOverride(15.0f)
							[
								SNew(SHorizontalBox)
							
									//spacer
									+ SHorizontalBox::Slot()
									.FillWidth(0.8f)
									[
										SNew(SSpacer)
									]
									+ SHorizontalBox::Slot()
									.FillWidth(0.2f)
									[
										SNew(SButton)
											.ButtonStyle(FAppStyle::Get(), "FlatButton.Success")
											.Text(FText::FromString("Compile"))
											.OnClicked_Lambda([this]() -> FReply
												{
													OnChuckCodeChanged.Broadcast();
													return FReply::Handled();
												})
									]
									+ SHorizontalBox::Slot()
									.FillWidth(0.2f)
									[
										SNew(SButton)
											.Text(FText::FromString("X"))
											.OnClicked_Lambda([this]() -> FReply
												{
													OnChuckWidgetUnfocus.Broadcast();
													return FReply::Handled();
												})
									]

							]
						
					]
			];


		return Hierarchy;
	}


	virtual void ReleaseSlateResources(bool bReleaseChildren) override
	{
		Super::ReleaseSlateResources(bReleaseChildren);

		CodeEditor.Reset();
	}

	void SetBoxSize(FIntPoint InSize)
	{
		if (CodeEditorBox.IsValid())
		{
			//CodeEditorBox->SetWidthOverride(InSize.X);
			//CodeEditorBox->SetHeightOverride(InSize.Y);
		}

	};

public:
	UFUNCTION(BlueprintCallable, Category = "ChucK")
	FText GetCode() const
	{
		if (ChuckCode)
		{
			return FText::FromString(ChuckCode->Code);
		}
		
		return INVTEXT("// sad but rue");
	}

	UFUNCTION(BlueprintCallable, Category = "ChucK")
	void SetCode(UChuckCode* InNewCodeObject)
	{
		ChuckCode = InNewCodeObject;
		if (CodeEditor.IsValid())
		{
			if (InNewCodeObject)
			{
			CodeEditor->SetText(FText::FromString(InNewCodeObject->Code));
			}
			else {
				CodeEditor->SetText(FText::FromString(ChuckCodeEditorWidget::DefaultCode));
			}
		}

		OnChuckCodeChanged.Broadcast();

	//CodeEditor->SetText(InCode);
	}

	//Use this to create a transient code object from an existing code file, useful to let the user modify chucks without affecting the original object
	UFUNCTION(BlueprintCallable, Category = "ChucK")
	void CopyCodeFromObject(UChuckCode* InCodeObject)
	{
		if (InCodeObject)
		{
			ChuckCode = NewObject<UChuckCode>(this);
			SetCode(InCodeObject);
		}
	}


	//TODO: remove?
	UFUNCTION(BlueprintCallable, Category = "ChucK")
	UChuckCode* SpawnNewChuckCodeObjectFromWidget()
	{
		FString Code = GetCode().ToString();
		UChuckCode* ChuckInstance = NewObject<UChuckCode>();
		ChuckInstance->Code = Code;
		//ChuckInstance->ChuckGuid = FGuid();
		return ChuckInstance;
	}
	
	UPROPERTY(BlueprintReadWrite, Category = "ChucK", meta = (BindWidget))
	UButton* CompileButton;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ChucK", meta = (ExposeOnSpawn = true, MultiLine = true))
	FString InitialCode = TEXT("<<<Hello World>>>;");


};
