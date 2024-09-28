// Copyright Epic Games, Inc. All Rights Reserved.

#include "SCodeEditableText.h"
#include "CodeEditorStyle.h"


void SCodeEditableText::Construct( const FArguments& InArgs )
{
	SMultiLineEditableText::Construct(
		SMultiLineEditableText::FArguments()
		.Font(FCodeEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("TextEditor.NormalText").Font)
		.TextStyle(&FCodeEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("TextEditor.NormalText"))
		.Text(InArgs._Text)
		.Marshaller(InArgs._Marshaller)
		.AutoWrapText(false)
		.Margin(0.0f)
		.HScrollBar(InArgs._HScrollBar)
		.VScrollBar(InArgs._VScrollBar)
		.OnTextChanged(InArgs._OnTextChanged)
	);
}

FReply SCodeEditableText::OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& InCharacterEvent)
{
	FReply Reply = FReply::Unhandled();

	const TCHAR Character = InCharacterEvent.GetCharacter();
	if(Character == TEXT('\t'))
	{
		if (!IsTextReadOnly())
		{
			FString String;
			String.AppendChar(Character);
			InsertTextAtCursor(String);
			Reply = FReply::Handled();
		}
		else
		{
			Reply = FReply::Unhandled();
		}
	}
	else
	{
		Reply = SMultiLineEditableText::OnKeyChar( MyGeometry, InCharacterEvent );
	}

	return Reply;
}

FReply SCodeEditableText::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	// If the key event is a tab key, we want to handle it ourselves
	UE_LOG(LogTemp, Warning, TEXT("Key"));
	if (InKeyEvent.GetKey() == EKeys::Tab)
	{
		if (!IsTextReadOnly())
		{
			FString String;
			String.AppendChar(TEXT('\t'));
			InsertTextAtCursor(String);
			return FReply::Handled();
		}
		else
		{
			return FReply::Unhandled();
		}
		
	}
	
	return FReply::Unhandled();
}
