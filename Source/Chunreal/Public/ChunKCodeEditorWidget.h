// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/Text/SMultiLineEditableText.h"
#include "ChucKSyntaxHighlighter.h"
#include "ChunKCodeEditorWidget.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class CHUNREAL_API UChuckCodeEditorWidget : public UUserWidget
{
	GENERATED_BODY()

	TSharedPtr<SMultiLineEditableText> CodeEditor;

	virtual TSharedRef<SWidget> RebuildWidget() override
	{
		SAssignNew(CodeEditor, SMultiLineEditableText)
			.Text(FText::FromString(TEXT("Hello World")))
			.OnTextChanged_Lambda([this](const FText& InText) {
			//FString Text = InText.ToString();
			//CodeEditor->SetText(FText::FromString(Text));
				})
			.Marshaller(FChucKSyntaxHighlighterMarshaller::Create());

		return CodeEditor.ToSharedRef();
	}


	virtual void ReleaseSlateResources(bool bReleaseChildren) override
	{
		Super::ReleaseSlateResources(bReleaseChildren);

		CodeEditor.Reset();
	}
	
};
