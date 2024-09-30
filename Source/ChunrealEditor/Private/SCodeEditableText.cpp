// Copyright Epic Games, Inc. All Rights Reserved.

#include "SCodeEditableText.h"
#include "CodeEditorStyle.h"


void SBkCodeEditableText::Construct( const FArguments& InArgs )
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

FReply SBkCodeEditableText::OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& InCharacterEvent)
{
	FReply Reply = FReply::Unhandled();


	const TCHAR Character = InCharacterEvent.GetCharacter();
	if(Character == TEXT('\t'))
	{
		if (!IsTextReadOnly())
		{
			// is shift pressed? if so, we need to try to remove a tab from the beginning of the line
			if (InCharacterEvent.IsShiftDown())
			{

				RemoveTabOnAllSelectedLines();
				Reply = FReply::Handled();
				return Reply;
			}

			
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

FReply SBkCodeEditableText::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	// If the key event is a tab key, we want to handle it ourselves to steal it from the editor

	if (InKeyEvent.GetKey() == EKeys::Tab)
	{
		if (!IsTextReadOnly())
		{
			return FReply::Handled();
		}
		
	}

	//if key is enter key, we want to handle it ourselves to steal it from the editor
	if (InKeyEvent.GetKey() == EKeys::Enter)
	{
		if (!IsTextReadOnly())
		{
			//insert new line
			FString String;
			String.AppendChar(TEXT('\n'));
			InsertTextAtCursor(String);
			return FReply::Handled();
		}
	}

	//key up down left right
	if (InKeyEvent.GetKey() == EKeys::Up || InKeyEvent.GetKey() == EKeys::Down || InKeyEvent.GetKey() == EKeys::Left || InKeyEvent.GetKey() == EKeys::Right)
	{
		return SMultiLineEditableText::OnKeyDown(MyGeometry, InKeyEvent);
	}

	//ctrl + c, ctrl + v, ctrl + x, ctrl+z, ctrl+y
	//if ctrl is pressed
	if (InKeyEvent.IsControlDown())
	{
		if (InKeyEvent.GetKey() == EKeys::C || InKeyEvent.GetKey() == EKeys::V || InKeyEvent.GetKey() == EKeys::X || InKeyEvent.GetKey() == EKeys::Z || InKeyEvent.GetKey() == EKeys::Y)
		{
			return SMultiLineEditableText::OnKeyDown(MyGeometry, InKeyEvent);
		}
	}

	
	return FReply::Unhandled();
	//return FReply::Unhandled();
}

void SBkCodeEditableText::RemoveTabOnAllSelectedLines()
{
	//get the selection range

	auto SelectionRange = GetSelection();
	if (SelectionRange.GetBeginning() != SelectionRange.GetEnd())
	{
		//get the start and end line
		int32 StartLine = SelectionRange.GetBeginning().GetLineIndex();
		int32 EndLine = SelectionRange.GetEnd().GetLineIndex();

		//iterate over the lines and insert a tab
		for (int32 LineIndex = StartLine; LineIndex <= EndLine; ++LineIndex)
		{
			FTextLocation Location(LineIndex, 0);
			GoTo(Location);
			//CodeEditableText->InsertTextAtCursor(FText::FromString("\t"));
			RemoveTabFromCurrentLine();
		}
	}
	else {
		RemoveTabFromCurrentLine();

	}
}

void SBkCodeEditableText::RemoveTabFromCurrentLine()
{
	//cursor should already be in the right spot
	FTextLocation CursorLocation = GetCursorLocation();
	FTextLocation StartOfLine(CursorLocation.GetLineIndex(), 0);
	
	//get the text of the line
	FString TextLine;
	GetTextLine(CursorLocation.GetLineIndex(), TextLine);

	//if text line is empty, return
	if (TextLine.Len() == 0)
	{
		return;
	}

	//we need to find the tab character preceding the cursor position, if any
	int32 TabIndex = -1;
	for (int32 i = CursorLocation.GetOffset() - 1; i >= 0; --i)
	{
		if (TextLine[i] == '\t')
		{
			TabIndex = i;
			break;
		}
	}

	//if we found a tab character, remove it by selecting it and deleting the selected text
	if (TabIndex != -1)
	{
		//remove the tab character
		FTextLocation TabStart(CursorLocation.GetLineIndex(), TabIndex);
		FTextLocation TabEnd(CursorLocation.GetLineIndex(), TabIndex + 1);
		
		SelectText(TabStart, TabEnd);

		DeleteSelectedText();

	}

}
