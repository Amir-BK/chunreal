// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/Text/SMultiLineEditableText.h"
#include "ChucKSyntaxHighlighter.h"
#include "SCodeEditableText.h"
#include "ChuckInstance.h"
#include "Components/Button.h"
#include "ChunKCodeEditorWidget.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class CHUNREAL_API UChuckCodeEditorWidget : public UUserWidget
{
	GENERATED_BODY()

	TSharedPtr<SBkCodeEditableText> CodeEditor;

	virtual TSharedRef<SWidget> RebuildWidget() override
	{
		SAssignNew(CodeEditor, SBkCodeEditableText)
			.Text(FText::FromString(*InitialCode))
			
			.Marshaller(FChucKSyntaxHighlighterMarshaller::Create());


		if (CompileButton)
		{
			//compile button icon
			auto CompileIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Recompile");
			CompileButton->SetStyle(
				FButtonStyle().SetNormal(*CompileIcon.GetIcon()));
		}

		return CodeEditor.ToSharedRef();
	}


	virtual void ReleaseSlateResources(bool bReleaseChildren) override
	{
		Super::ReleaseSlateResources(bReleaseChildren);

		CodeEditor.Reset();
	}

public:
	UFUNCTION(BlueprintCallable, Category = "ChucK")
	FText GetCode() const
	{
		return CodeEditor->GetText();
	}

	UFUNCTION(BlueprintCallable, Category = "ChucK")
	void SetCode(const FText& InCode)
	{
		CodeEditor->SetText(InCode);
	}

	UFUNCTION(BlueprintCallable, Category = "ChucK")
	UChuckProcessor* SpawnChuckInstanceFromCode()
	{
		FString Code = GetCode().ToString();
		UChuckProcessor* ChuckInstance = NewObject<UChuckProcessor>();
		ChuckInstance->Code = Code;
		ChuckInstance->ChuckGuid = FGuid::NewGuid();
		return ChuckInstance;
	}
	
	UPROPERTY(BlueprintReadWrite, Category = "ChucK", meta = (BindWidget))
	UButton* CompileButton;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ChucK", meta = (ExposeOnSpawn = true, MultiLine = true))
	FString InitialCode = TEXT("<<<Hello World>>>;");


};
