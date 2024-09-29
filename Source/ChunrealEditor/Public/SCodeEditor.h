// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widgets/SCompoundWidget.h"

class SBkCodeEditableText;
class SScrollBar;

class SCodeEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCodeEditor) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, class UCodeProjectItem* InCodeProjectItem);

	bool Save() const;

	bool CanSave() const;

	void GotoLineAndColumn(int32 LineNumber, int32 ColumnNumber);

	void InsertTextAtCursor(const FString& Text);

	void InsertTabOnAllSelectedLines();

	void RemoveTabOnAllSelectedLines();

private:
	void OnTextChanged(const FText& NewText);

protected:
	class UCodeProjectItem* CodeProjectItem;

	TSharedPtr<SScrollBar> HorizontalScrollbar;
	TSharedPtr<SScrollBar> VerticalScrollbar;

	TSharedPtr<class SBkCodeEditableText> CodeEditableText;

	mutable bool bDirty;
};
