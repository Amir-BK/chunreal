// Copyright Epic Games, Inc. All Rights Reserved.

#include "SCodeEditableText.h"
#include "Widgets/Text/SlateEditableTextLayout.h"
#include "CodeEditorStyle.h"


void SBkCodeEditableText::Construct( const FArguments& InArgs )
{
	SChunrealMultiLineEditableText::Construct(
		SChunrealMultiLineEditableText::FArguments()
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
		Reply = SChunrealMultiLineEditableText::OnKeyChar( MyGeometry, InCharacterEvent );
	}

	return Reply;
}

FReply SBkCodeEditableText::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	/*
	* 
	// take 'delete' key as handled
	if (InKeyEvent.GetKey() == EKeys::Delete)
	{
		if (!IsTextReadOnly())
		{
			UE_LOG(LogTemp, Warning, TEXT("Delete key pressed"));
			DeleteSelectedText();
			//DeleteSelectedText();
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

	//key up down 
	if (InKeyEvent.GetKey() == EKeys::Up || InKeyEvent.GetKey() == EKeys::Down)
	{
		return SChunrealMultiLineEditableText::OnKeyDown(MyGeometry, InKeyEvent);
	}

	// if left right we also want to check if ctrl is checked, if it is we want to do simple caret movement
	if (InKeyEvent.GetKey() == EKeys::Left || InKeyEvent.GetKey() == EKeys::Right)
	{
		if (!InKeyEvent.IsControlDown())
		{
			return SChunrealMultiLineEditableText::OnKeyDown(MyGeometry, InKeyEvent);
		}
		else {

			//caret movement
			if (InKeyEvent.GetKey() == EKeys::Left)
			{
				CaretMoveLeft();
			}
			else
			{
				//caret move right
			}

			return FReply::Handled();
		}
	}


	//ctrl + c, ctrl + v, ctrl + x, ctrl+z, ctrl+y
	//if ctrl is pressed
	if (InKeyEvent.IsControlDown())
	{
		if (InKeyEvent.GetKey() == EKeys::C || InKeyEvent.GetKey() == EKeys::V || InKeyEvent.GetKey() == EKeys::X ||
			InKeyEvent.GetKey() == EKeys::Z || InKeyEvent.GetKey() == EKeys::Y ||  InKeyEvent.GetKey() == EKeys::A)
		{
			return SChunrealMultiLineEditableText::OnKeyDown(MyGeometry, InKeyEvent);
		}
	}

	*/
	// If the key event is a tab key, we want to handle it ourselves to steal it from the layout class, for some reason the layout class is not handling it correctly

	if (InKeyEvent.GetKey() == EKeys::Tab)
	{
		if (!IsTextReadOnly())
		{

			return FReply::Handled();
		}

	}
	
	return SChunrealMultiLineEditableText::OnKeyDown(MyGeometry, InKeyEvent);
	//return FReply::Unhandled();
}

FReply SBkCodeEditableText::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	//print token under mouse

	//get mouse location and convert it to text location
	FVector2D MousePosition = MouseEvent.GetScreenSpacePosition();
	FVector2D LocalMousePosition = MyGeometry.AbsoluteToLocal(MousePosition);


	EditableTextLayout->HighlightTokenUnderCursor(LocalMousePosition);
	

	return SChunrealMultiLineEditableText::OnMouseMove(MyGeometry, MouseEvent);
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

void SBkCodeEditableText::CaretMoveLeft()
{
	//from current cursor location, if on a token, move to the beginning of the token, if not on a token, move to the end of the previous token
	//if no token in line, move to the end of the previous line, if at the beginning of the first line, do nothing.

	FTextLocation CursorLocation = GetCursorLocation();
	FString TextLine;
	GetTextLine(CursorLocation.GetLineIndex(), TextLine);

	//if cursor is at the beginning of the line, move to the end of the previous line
	if (CursorLocation.GetOffset() == 0)
	{
		if (CursorLocation.GetLineIndex() > 0)
		{
			FTextLocation NewLocation(CursorLocation.GetLineIndex() - 1, 0);
			GoTo(NewLocation);
		}
	}
	else
	{
		//if cursor is not at the beginning of the line, move to the beginning of the token
		//if cursor is on a token, move to the beginning of the token
		//if cursor is not on a token, move to the end of the previous token
		//if no token in line, move to the end of the previous line

		//if cursor is on a token, move to the beginning of the token
		if (FChar::IsIdentifier(TextLine[CursorLocation.GetOffset() - 1]))
		{
			//move to the beginning of the token
			int32 TokenStart = CursorLocation.GetOffset() - 1;
			while (TokenStart > 0 && FChar::IsIdentifier(TextLine[TokenStart - 1]))
			{
				--TokenStart;
			}

			FTextLocation NewLocation(CursorLocation.GetLineIndex(), TokenStart);
			GoTo(NewLocation);
		}
		else
		{
			//if cursor is not on a token, move to the end of the previous token
			//if no token in line, move to the end of the previous line
			//if cursor is at the beginning of the line, move to the end of the previous line, do nothing
			//if cursor is at the beginning of the first line, do nothing

			//if cursor is at the beginning of the line, move to the end of the previous line
			if (CursorLocation.GetOffset() == 0)
			{
				if (CursorLocation.GetLineIndex() > 0)
				{
					FTextLocation NewLocation(CursorLocation.GetLineIndex() - 1, 0);
					GoTo(NewLocation);
				}
			}
			else
			{
				//if cursor is not at the beginning of the line, move to the end of the previous token
				//if no token in line, move to the end of the previous line

				//if cursor is not at the
				//if cursor is not at the beginning of the line, move to the end of the previous token
				//if no token in line, move to the end of the previous line
				//if at the beginning of the first line, do nothing.
			}
		}
	}

}
