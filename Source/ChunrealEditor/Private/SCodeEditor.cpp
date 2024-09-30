// Copyright Epic Games, Inc. All Rights Reserved.

#include "SCodeEditor.h"
#include "Misc/FileHelper.h"
#include "Widgets/Layout/SGridPanel.h"
#include "CodeProjectItem.h"
#include "CPPRichTextSyntaxHighlighterTextLayoutMarshaller.h"
#include "ChucKSyntaxHighlighter.h"
#include "SCodeEditableText.h"


#define LOCTEXT_NAMESPACE "CodeEditor"


void SCodeEditor::Construct(const FArguments& InArgs, UCodeProjectItem* InCodeProjectItem)
{
	bDirty = false;

	check(InCodeProjectItem);
	CodeProjectItem = InCodeProjectItem;

	FString FileText = "File Loading, please wait";
	FFileHelper::LoadFileToString(FileText, *InCodeProjectItem->Path);
	//TODO: probably check the type of the file even though we only care about chucks for now
	TSharedRef<FChucKSyntaxHighlighterMarshaller> RichTextMarshaller = FChucKSyntaxHighlighterMarshaller::Create();

	HorizontalScrollbar = 
		SNew(SScrollBar)
		.Orientation(Orient_Horizontal)
		.Thickness(FVector2D(14.0f, 14.0f));

	VerticalScrollbar = 
		SNew(SScrollBar)
		.Orientation(Orient_Vertical)
		.Thickness(FVector2D(14.0f, 14.0f));

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCodeEditorStyle::Get().GetBrush("TextEditor.Border"))
		[
			SNew(SGridPanel)
			.FillColumn(0, 1.0f)
			.FillRow(0, 1.0f)
			+SGridPanel::Slot(0, 0)
			[
				SAssignNew(CodeEditableText, SBkCodeEditableText)
				.Text(FText::FromString(FileText))
				.Marshaller(RichTextMarshaller)
				.HScrollBar(HorizontalScrollbar)
				.VScrollBar(VerticalScrollbar)
				.OnTextChanged(this, &SCodeEditor::OnTextChanged)
			]
			+SGridPanel::Slot(1, 0)
			[
				VerticalScrollbar.ToSharedRef()
			]
			+SGridPanel::Slot(0, 1)
			[
				HorizontalScrollbar.ToSharedRef()
			]
		]
	];
}

void SCodeEditor::OnTextChanged(const FText& NewText)
{
	bDirty = true;
}

bool SCodeEditor::Save() const
{
	if(bDirty)
	{
		bool bResult = FFileHelper::SaveStringToFile(CodeEditableText->GetText().ToString(), *CodeProjectItem->Path);
		if(bResult)
		{
			bDirty = false;
		}

		return bResult;
	}
	return true;
}

bool SCodeEditor::CanSave() const
{
	return bDirty;
}

void SCodeEditor::GotoLineAndColumn(int32 LineNumber, int32 ColumnNumber)
{
	FTextLocation Location(LineNumber, ColumnNumber);
	CodeEditableText->GoTo(Location);
	CodeEditableText->ScrollTo(Location);
}

void SCodeEditor::InsertTextAtCursor(const FString& Text)
{
	//CodeEditableText->InsertTextAtCursor(FText::FromString(Text));
	InsertTabOnAllSelectedLines();
}

void SCodeEditor::InsertTabOnAllSelectedLines()
{
	//get the selection range
	auto SelectionRange = CodeEditableText->GetSelection();
	if (SelectionRange.GetBeginning() != SelectionRange.GetEnd())
	{
		//get the start and end line
		int32 StartLine = SelectionRange.GetBeginning().GetLineIndex();
		int32 EndLine = SelectionRange.GetEnd().GetLineIndex();

		//iterate over the lines and insert a tab
		for (int32 LineIndex = StartLine; LineIndex <= EndLine; ++LineIndex)
		{
			FTextLocation Location(LineIndex, 0);
			CodeEditableText->GoTo(Location);
			CodeEditableText->InsertTextAtCursor(FText::FromString("\t"));
		}
	}
}

#undef LOCTEXT_NAMESPACE
