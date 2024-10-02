// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Attribute.h"
#include "Layout/Geometry.h"
#include "Input/Reply.h"
#include "Widgets/SWidget.h"
#include "Layout/Margin.h"
#include "Styling/SlateTypes.h"
#include "Framework/Application/IMenu.h"
#include "Widgets/Input/IVirtualKeyboardEntry.h"
#include "IChunrealSlateEditableTextWidget.h"
//#include "Widgets/Text/SlateEditableTextLayout.h"
#include "Framework/Text/ITextLayoutMarshaller.h"
#include "Framework/Text/TextLineHighlight.h"
#include "Framework/Text/IRun.h"
#include "Framework/Text/TextLayout.h"
#include "Widgets/Text/SlateEditableTextTypes.h"
#include "Widgets/Input/SEditableText.h"
//#include "Framework/Text/SlateTextLayoutFactory.h"
#include "GenericPlatform/ITextInputMethodSystem.h"
#include <ChunrealSlateTextLayout.h>

class FArrangedChildren;
class FExtender;
class FPaintArgs;
class FSlateWindowElementList;
class FSlateTextBlockLayout;
class FUICommandList;
class IBreakIterator;
class SWindow;
enum class ETextShapingMethod : uint8;

DECLARE_DELEGATE_RetVal_TwoParams(TSharedRef<FChunrealSlateTextLayout>, FCreateChunrealSlateTextLayout, SWidget*, const FTextBlockStyle&);

/** Class to handle the cached layout of SEditableText/SMultiLineEditableText by proxying around a FTextLayout */
class FChunrealSlateEditableTextLayout
{

	FTextSelection HighlightedToken = FTextSelection();
	FString HighLightTokenString = FString();

public:
	CHUNREAL_API FChunrealSlateEditableTextLayout(IChunrealSlateEditableTextWidget& InOwnerWidget, const TAttribute<FText>& InInitialText, FTextBlockStyle InTextStyle, const TOptional<ETextShapingMethod> InTextShapingMethod, const TOptional<ETextFlowDirection> InTextFlowDirection, const FCreateChunrealSlateTextLayout& InCreateSlateTextLayout, TSharedRef<ITextLayoutMarshaller> InTextMarshaller, TSharedRef<ITextLayoutMarshaller> InHintTextMarshaller);
	CHUNREAL_API ~FChunrealSlateEditableTextLayout();

	CHUNREAL_API void SetText(const TAttribute<FText>& InText);
	CHUNREAL_API FText GetText() const;

	CHUNREAL_API void SetHintText(const TAttribute<FText>& InHintText);
	CHUNREAL_API FText GetHintText() const;

	CHUNREAL_API void SetSearchText(const TAttribute<FText>& InSearchText);
	CHUNREAL_API FText GetSearchText() const;
	
	/** Get the index of the search result (0 if none) */
	CHUNREAL_API int32 GetSearchResultIndex() const;
	
	/** Get the total number of search results (0 if none) */
	CHUNREAL_API int32 GetNumSearchResults() const;

	CHUNREAL_API void SetTextStyle(const FTextBlockStyle& InTextStyle);
	CHUNREAL_API const FTextBlockStyle& GetTextStyle() const;

	/** Get the number of Text Lines */
	CHUNREAL_API int32 GetTextLineCount();

	/** Set the brush to use when drawing the cursor */
	CHUNREAL_API void SetCursorBrush(const TAttribute<const FSlateBrush*>& InCursorBrush);

	/** Set the brush to use when drawing the composition highlight */
	CHUNREAL_API void SetCompositionBrush(const TAttribute<const FSlateBrush*>& InCompositionBrush);

	/** Get the plain text string without rich-text formatting */
	CHUNREAL_API FText GetPlainText() const;

	/**
	 * Sets the current editable text for this text block
	 * Note: Doesn't update the value of BoundText, nor does it call OnTextChanged
	 *
	 * @param TextToSet	The new text to set in the internal TextLayout
	 * @param bForce	True to force the update, even if the text currently matches what's in the TextLayout
	 *
	 * @return true if the text was updated, false if the text wasn't update (because it was already up-to-date)
	 */
	CHUNREAL_API bool SetEditableText(const FText& TextToSet, const bool bForce = false);

	/**
	 * Gets the current editable text for this text block
	 * Note: We don't store text in this form (it's stored as lines in the text layout) so every call to this function has to reconstruct it
	 */
	CHUNREAL_API FText GetEditableText() const;

	/** Get the currently selected text */
	CHUNREAL_API FText GetSelectedText() const;

	/** Get the current selection*/
	CHUNREAL_API FTextSelection GetSelection() const;
	
	/** Set the text shaping method that the internal text layout should use */
	CHUNREAL_API void SetTextShapingMethod(const TOptional<ETextShapingMethod>& InTextShapingMethod);

	/** Set the text flow direction that the internal text layout should use */
	CHUNREAL_API void SetTextFlowDirection(const TOptional<ETextFlowDirection>& InTextFlowDirection);

	/** Set the wrapping to use for this document */
	CHUNREAL_API void SetTextWrapping(const TAttribute<float>& InWrapTextAt, const TAttribute<bool>& InAutoWrapText, const TAttribute<ETextWrappingPolicy>& InWrappingPolicy);

	/** Set whether text wraps onto a new line when it's length exceeds this width; if this value is zero or negative, no wrapping occurs */
	CHUNREAL_API void SetWrapTextAt(const TAttribute<float>& InWrapTextAt);

	/** Set whether to wrap text automatically based on the widget's computed horizontal space */
	CHUNREAL_API void SetAutoWrapText(const TAttribute<bool>& InAutoWrapText);

	/** Set the wrapping policy to use */
	CHUNREAL_API void SetWrappingPolicy(const TAttribute<ETextWrappingPolicy>& InWrappingPolicy);

	/** Set the amount of blank space left around the edges of text area */
	CHUNREAL_API void SetMargin(const TAttribute<FMargin>& InMargin);

	/** Set how the text should be aligned with the margin */
	CHUNREAL_API void SetJustification(const TAttribute<ETextJustify::Type>& InJustification);

	/** Set the amount to scale each lines height by */
	CHUNREAL_API void SetLineHeightPercentage(const TAttribute<float>& InLineHeightPercentage);

	/** Set whether to leave extra space below the last line due to line height */
	CHUNREAL_API void SetApplyLineHeightToBottomLine(const TAttribute<bool>& InApplyLineHeightToBottomLine);

	/** Set the text overflow policy that should be used to determine what happens to clipped text */
	CHUNREAL_API void SetOverflowPolicy(TOptional<ETextOverflowPolicy> InOverflowPolicy);

	/** Set the information used to help identify who owns this text layout in the case of an error */
	CHUNREAL_API void SetDebugSourceInfo(const TAttribute<FString>& InDebugSourceInfo);

	/** Get the virtual keyboard handler for this text layout */
	CHUNREAL_API TSharedRef<IVirtualKeyboardEntry> GetVirtualKeyboardEntry() const;

	/** Get the IME context for this text layout */
	CHUNREAL_API TSharedRef<ITextInputMethodContext> GetTextInputMethodContext() const;

	/** Register and activate the IME context for this text layout */
	CHUNREAL_API void EnableTextInputMethodContext();

	/** Refresh this editable text immediately, rather than wait for the usual caching mechanisms to take affect on the text Tick */
	CHUNREAL_API bool Refresh();

	/** Force the text layout to be updated from the marshaller */
	CHUNREAL_API void ForceRefreshTextLayout(const FText& CurrentText);

	/** Begin a new text search (this is called automatically when BoundSearchText changes) */
	CHUNREAL_API void BeginSearch(const FText& InSearchText, const ESearchCase::Type InSearchCase = ESearchCase::IgnoreCase, const bool InReverse = false);

	/** Advance the current search to the next match (does nothing if not currently searching) */
	CHUNREAL_API void AdvanceSearch(const bool InReverse = false);

	/** Update the horizontal scroll amount from the given fraction */
	CHUNREAL_API UE::Slate::FDeprecateVector2DResult SetHorizontalScrollFraction(const float InScrollOffsetFraction);

	/** Update the vertical scroll amount from the given fraction */
	CHUNREAL_API UE::Slate::FDeprecateVector2DResult SetVerticalScrollFraction(const float InScrollOffsetFraction);

	/** Set the absolute scroll offset value */
	CHUNREAL_API UE::Slate::FDeprecateVector2DResult SetScrollOffset(const UE::Slate::FDeprecateVector2DParameter& InScrollOffset, const FGeometry& InGeometry);

	/** Get the absolute scroll offset value */
	CHUNREAL_API UE::Slate::FDeprecateVector2DResult GetScrollOffset() const;

	/** Returns the computed wrap location for this layout */
	CHUNREAL_API float GetComputedWrappingWidth() const;

	/** Returns whether or not we are auto wrapping text */
	CHUNREAL_API bool GetAutoWrapText() const;

	/** Called when our parent widget receives focus */
	CHUNREAL_API bool HandleFocusReceived(const FFocusEvent& InFocusEvent);

	/** Called when our parent widget loses focus */
	CHUNREAL_API bool HandleFocusLost(const FFocusEvent& InFocusEvent);

	/** Called to handle an OnKeyChar event from our parent widget */
	CHUNREAL_API FReply HandleKeyChar(const FCharacterEvent& InCharacterEvent);

	/** Called to handle an OnKeyDown event from our parent widget */
	CHUNREAL_API FReply HandleKeyDown(const FKeyEvent& InKeyEvent);

	/** Called to handle an OnKeyUp event from our parent widget */
	CHUNREAL_API FReply HandleKeyUp(const FKeyEvent& InKeyEvent);

	/** Called to handle an OnMouseButtonDown event from our parent widget */
	CHUNREAL_API FReply HandleMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& InMouseEvent);

	/** Called to handle an OnMouseButtonUp event from our parent widget */
	CHUNREAL_API FReply HandleMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& InMouseEvent);

	/** Called to handle an OnMouseMove event from our parent widget */
	CHUNREAL_API FReply HandleMouseMove(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent);

	/** Called to handle an OnMouseButtonDoubleClick event from our parent widget */
	CHUNREAL_API FReply HandleMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent);

	/** Called to handle an escape action acting on the current selection */
	CHUNREAL_API bool HandleEscape();

	/** Called to handle a backspace action acting on the current selection or at the cursor position */
	CHUNREAL_API bool HandleBackspace();

	/** Called to handle a delete action acting on the current selection or at the cursor position */
	CHUNREAL_API bool HandleDelete();

	/** Called to handle a typing a character on the current selection or at the cursor position */
	CHUNREAL_API bool HandleTypeChar(const TCHAR InChar);

	/** Called to handle a carriage return action acting on the current selection or at the cursor position */
	CHUNREAL_API bool HandleCarriageReturn(bool isRepeat);

	/** Are we able to delete the currently selected text? */
	CHUNREAL_API bool CanExecuteDelete() const;

	/** Delete any currently selected text */
	CHUNREAL_API void DeleteSelectedText();

	/** Query to see if any text is selected within the document */
	CHUNREAL_API bool AnyTextSelected() const;

	/** Query to see if the text under the given position is currently selected */
	CHUNREAL_API bool IsTextSelectedAt(const FGeometry& MyGeometry, const UE::Slate::FDeprecateVector2DParameter& ScreenSpacePosition) const;

	/** Query to see if the text under the given position is currently selected (the position is local to the text layout space) */
	CHUNREAL_API bool IsTextSelectedAt(const UE::Slate::FDeprecateVector2DParameter& InLocalPosition) const;

	/** Are we able to execute the "Select All" command? */
	CHUNREAL_API bool CanExecuteSelectAll() const;

	/** Select all the text in the document */
	CHUNREAL_API void SelectAllText();

	/** Select the word under the given position */
	CHUNREAL_API void SelectWordAt(const FGeometry& MyGeometry, const UE::Slate::FDeprecateVector2DParameter& ScreenSpacePosition);

	/** Select the word under the given position (the position is local to the text layout space) */
	CHUNREAL_API void SelectWordAt(const UE::Slate::FDeprecateVector2DParameter& InLocalPosition);

	/** Get the word under the given position */
	CHUNREAL_API void HighlightTokenUnderCursor(const UE::Slate::FDeprecateVector2DParameter& InLocalPosition);

	/** Select a block of text */
	CHUNREAL_API void SelectText(const FTextLocation& InSelectionStart, const FTextLocation& InCursorLocation);

	/** Clear the active text selection */
	CHUNREAL_API void ClearSelection();

	/** Are we able to cut the currently selected text? */
	CHUNREAL_API bool CanExecuteCut() const;

	/** Cut the currently selected text and place it on the clipboard */
	CHUNREAL_API void CutSelectedTextToClipboard();

	/** Are we able to copy the currently selected text? */
	CHUNREAL_API bool CanExecuteCopy() const;

	/** Copy the currently selected text and place it on the clipboard */
	CHUNREAL_API void CopySelectedTextToClipboard();

	/** Are we able to paste the text from the clipboard into this document? */
	CHUNREAL_API bool CanExecutePaste() const;

	/** Paste the text from the clipboard into this document */
	CHUNREAL_API void PasteTextFromClipboard();

	/** Insert the given text at the current cursor position, correctly taking into account new line characters */
	CHUNREAL_API void InsertTextAtCursor(const FString& InString);

	/** Insert the given run at the current cursor position */
	CHUNREAL_API void InsertRunAtCursor(TSharedRef<IRun> InRun);

	/** Move the cursor in the document using the specified move method */
	CHUNREAL_API bool MoveCursor(const FChunrealMoveCursor& InArgs);

	/** Move the cursor to the given location in the document (will also scroll to this point) */
	CHUNREAL_API void GoTo(const FTextLocation& NewLocation);

	/** Move the cursor specified location */
	CHUNREAL_API void GoTo(const EChunrealTextLocation NewLocation);

	/** Jump the cursor to the given location in the document */
	CHUNREAL_API void JumpTo(EChunrealTextLocation JumpLocation, EChunrealCursorAction Action);

	/** Scroll to the given location in the document (without moving the cursor) */
	CHUNREAL_API void ScrollTo(const FTextLocation& NewLocation);

	/** Scroll to the given location in the document (without moving the cursor) */
	CHUNREAL_API void ScrollTo(const EChunrealTextLocation NewLocation);

	/** Update the active cursor highlight based on the state of the text layout */
	CHUNREAL_API void UpdateCursorHighlight();

	/** Remove any active cursor highlights */
	CHUNREAL_API void RemoveCursorHighlight();

	/** Update the preferred offset of the cursor, based on the current state of the text layout */
	CHUNREAL_API void UpdatePreferredCursorScreenOffsetInLine();

	/** Apply the given style to the currently selected text (or insert a new run at the current cursor position if no text is selected) */
	CHUNREAL_API void ApplyToSelection(const FRunInfo& InRunInfo, const FTextBlockStyle& InStyle);

	/** Get the run currently under the cursor, or null if there is no run currently under the cursor */
	CHUNREAL_API TSharedPtr<const IRun> GetRunUnderCursor() const;

	/** Get the runs currently that are current selected, some of which may be only partially selected */
	CHUNREAL_API TArray<TSharedRef<const IRun>> GetSelectedRuns() const;

	/** Get the interaction position of the cursor (where to insert, delete, etc, text from/to) */
	CHUNREAL_API FTextLocation GetCursorLocation() const;

	/**
	 * Given a location and a Direction to offset, return a new location.
	 *
	 * @param Location    Cursor location from which to offset
	 * @param Direction   Positive means right, negative means left.
	 */
	CHUNREAL_API FTextLocation TranslatedLocation(const FTextLocation& CurrentLocation, int8 Direction) const;

	/**
	 * Given a location and a Direction to offset, return a new location.
	 *
	 * @param Location              Cursor location from which to offset
	 * @param NumLinesToMove        Number of lines to move in a given direction. Positive means down, negative means up.
	 * @param GeometryScale         Geometry DPI scale at which the widget is being rendered
	 * @param OutCursorPosition     Fill with the updated cursor position.
	 * @param OutCursorAlignment    Optionally fill with a new cursor alignment (will be auto-calculated if not set).
	 */
	CHUNREAL_API void TranslateLocationVertical(const FTextLocation& Location, int32 NumLinesToMove, float GeometryScale, FTextLocation& OutCursorPosition, TOptional<SlateEditableTextTypes::ECursorAlignment>& OutCursorAlignment) const;

	/** Find the closest word boundary */
	CHUNREAL_API FTextLocation ScanForWordBoundary(const FTextLocation& Location, int8 Direction) const;

	/** Get the character at Location */
	CHUNREAL_API TCHAR GetCharacterAt(const FTextLocation& Location) const;

	/** Are we at the beginning of all the text. */
	CHUNREAL_API bool IsAtBeginningOfDocument(const FTextLocation& Location) const;

	/** Are we at the end of all the text. */
	CHUNREAL_API bool IsAtEndOfDocument(const FTextLocation& Location) const;

	/** Is this location the beginning of a line */
	CHUNREAL_API bool IsAtBeginningOfLine(const FTextLocation& Location) const;

	/** Is this location the end of a line. */
	CHUNREAL_API bool IsAtEndOfLine(const FTextLocation& Location) const;

	/** Are we currently at the beginning of a word */
	CHUNREAL_API bool IsAtWordStart(const FTextLocation& Location) const;

	/** Restores the text to the original state */
	CHUNREAL_API void RestoreOriginalText();

	/** Returns whether the current text varies from the original */
	CHUNREAL_API bool HasTextChangedFromOriginal() const;

	/** Called to begin an undoable editable text transaction */
	CHUNREAL_API void BeginEditTransation();

	/** Called to end an undoable editable text transaction */
	CHUNREAL_API void EndEditTransaction();

	/** Push the given undo state onto the undo stack */
	CHUNREAL_API void PushUndoState(const SlateEditableTextTypes::FUndoState& InUndoState);

	/** Clear the current undo stack */
	CHUNREAL_API void ClearUndoStates();

	/** Create an undo state that reflects the current state of the document */
	CHUNREAL_API void MakeUndoState(SlateEditableTextTypes::FUndoState& OutUndoState);

	/** Are we currently able to execute an undo action? */
	CHUNREAL_API bool CanExecuteUndo() const;
	
	/** Execute an undo action */
	CHUNREAL_API void Undo();

	/** Are we currently able to execute a redo action? */
	CHUNREAL_API bool CanExecuteRedo() const;

	/** Execute a redo action */
	CHUNREAL_API void Redo();

	CHUNREAL_API void SaveText(const FText& TextToSave);

	CHUNREAL_API void LoadText();

	CHUNREAL_API bool ComputeVolatility() const;

	CHUNREAL_API void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime);

	CHUNREAL_API int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled);

	CHUNREAL_API void CacheDesiredSize(float LayoutScaleMultiplier);

	CHUNREAL_API FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const;

	CHUNREAL_API FChildren* GetChildren();

	CHUNREAL_API void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const;

	CHUNREAL_API UE::Slate::FDeprecateVector2DResult GetSize() const;

	CHUNREAL_API TSharedRef<SWidget> BuildDefaultContextMenu(const TSharedPtr<FExtender>& InMenuExtender) const;

	CHUNREAL_API bool HasActiveContextMenu() const;

	/**
	 * Fill OutTextLine with the text line where the current cursor location is at
	 *
	 * @param OutTextLine   FString of the line
	 */
	CHUNREAL_API void GetCurrentTextLine(FString& OutTextLine) const;

	/**
	 * Fill OutTextLine with the text line at the specified index
	 *
	 * @param InLineIndex   Index of the line
	 * @param OutTextLine   FString of the line
	 */
	CHUNREAL_API void GetTextLine(const int32 InLineIndex, FString& OutTextLine) const;

private:
	/** Insert the given text at the current cursor position, correctly taking into account new line characters */
	CHUNREAL_API void InsertTextAtCursorImpl(const FString& InString);

	/** Insert a new-line at the current cursor position */
	CHUNREAL_API void InsertNewLineAtCursorImpl();

	/** Implementation of Refresh that actually updates the layout. Optionally takes text to set, or will use the current editable text if none if provided */
	CHUNREAL_API bool RefreshImpl(const FText* InTextToSet, const bool bForce = false);

	/** Create a text or password run using the given text and style */
	CHUNREAL_API TSharedRef<IRun> CreateTextOrPasswordRun(const FRunInfo& InRunInfo, const TSharedRef<const FString>& InText, const FTextBlockStyle& InStyle);

	/** Called when the active context menu is closed */
	CHUNREAL_API void OnContextMenuClosed(TSharedRef<IMenu> Menu);

private:
	/** Virtual keyboard handler for an editable text layout */
	friend class FVirtualKeyboardEntry;
	class FVirtualKeyboardEntry : public IVirtualKeyboardEntry
	{
	public:
		static TSharedRef<FVirtualKeyboardEntry> Create(FChunrealSlateEditableTextLayout& InOwnerLayout);

		virtual void SetTextFromVirtualKeyboard(const FText& InNewText, ETextEntryType TextEntryType) override;
		virtual void SetSelectionFromVirtualKeyboard(int InSelStart, int InSelEnd) override;
		virtual bool GetSelection(int& OutSelStart, int& OutSelEnd) override;

		virtual FText GetText() const override;
		virtual FText GetHintText() const override;
		virtual EKeyboardType GetVirtualKeyboardType() const override;
		virtual FVirtualKeyboardOptions GetVirtualKeyboardOptions() const override;
		virtual bool IsMultilineEntry() const override;

	private:
		FVirtualKeyboardEntry(FChunrealSlateEditableTextLayout& InOwnerLayout);
		FChunrealSlateEditableTextLayout* OwnerLayout;
	};

private:
	/**
	 * Note: The IME interface for the multiline editable text uses the pre-flowed version of the string since the IME APIs are designed to work with flat strings
	 *		 This means we have to do a bit of juggling to convert between the two
	 */
	friend class FTextInputMethodContext;
	class FTextInputMethodContext : public ITextInputMethodContext
	{
	public:
		static TSharedRef<FTextInputMethodContext> Create(FChunrealSlateEditableTextLayout& InOwnerLayout);

		void CacheWindow();

		FORCEINLINE void KillContext()
		{
			OwnerLayout = nullptr;
			bIsComposing = false;
		}

		FORCEINLINE FTextRange GetCompositionRange() const
		{
			return FTextRange(CompositionBeginIndex, CompositionBeginIndex + CompositionLength);
		}

		bool UpdateCachedGeometry(const FGeometry& InAllottedGeometry)
		{
			if (CachedGeometry != InAllottedGeometry)
			{
				CachedGeometry = InAllottedGeometry;
				return true;
			}
			return false;
		}

		virtual bool IsComposing() override;
		virtual bool IsReadOnly() override;
		virtual uint32 GetTextLength() override;
		virtual void GetSelectionRange(uint32& BeginIndex, uint32& Length, ECaretPosition& CaretPosition) override;
		virtual void SetSelectionRange(const uint32 BeginIndex, const uint32 Length, const ECaretPosition CaretPosition) override;
		virtual void GetTextInRange(const uint32 BeginIndex, const uint32 Length, FString& OutString) override;
		virtual void SetTextInRange(const uint32 BeginIndex, const uint32 Length, const FString& InString) override;
		virtual int32 GetCharacterIndexFromPoint(const FVector2D& Point) override;
		virtual bool GetTextBounds(const uint32 BeginIndex, const uint32 Length, FVector2D& Position, FVector2D& Size) override;
		virtual void GetScreenBounds(FVector2D& Position, FVector2D& Size) override;
		virtual TSharedPtr<FGenericWindow> GetWindow() override;
		virtual void BeginComposition() override;
		virtual void UpdateCompositionRange(const int32 InBeginIndex, const uint32 InLength) override;
		virtual void EndComposition() override;

	private:
		FTextInputMethodContext(FChunrealSlateEditableTextLayout& InOwnerLayout);
		FChunrealSlateEditableTextLayout* OwnerLayout;
		TWeakPtr<SWindow> CachedParentWindow;

		FGeometry CachedGeometry;
		bool bIsComposing;
		int32 CompositionBeginIndex;
		uint32 CompositionLength;
	};

private:
	/** Pointer to the interface for our owner widget */
	IChunrealSlateEditableTextWidget* OwnerWidget;

	/** The iterator to use to detect grapheme cluster boundaries */
	TSharedPtr<IBreakIterator> GraphemeBreakIterator;

	/** The marshaller used to get/set the BoundText text to/from the text layout. */
	TSharedPtr<ITextLayoutMarshaller> Marshaller;

	/** The marshaller used to get/set the HintText text to/from the text layout. */
	TSharedPtr<ITextLayoutMarshaller> HintMarshaller;

	/** Delegate used to create internal text layouts. */
	FCreateChunrealSlateTextLayout CreateSlateTextLayout;

	/** In control of the layout and wrapping of the BoundText */
	TSharedPtr<FChunrealSlateTextLayout> TextLayout;

	/** In control of the layout and wrapping of the HintText */
	TUniquePtr<FSlateTextBlockLayout> HintTextLayout;

	/** Default style used by the TextLayout */
	FTextBlockStyle TextStyle;

	/** Style used to draw the hint text (only valid when HintTextLayout is set) */
	FTextBlockStyle HintTextStyle;

	/** The text displayed in this text block */
	TAttribute<FText> BoundText;

	/** The state of BoundText last Tick() (used to allow updates when the text is changed) */
	FTextSnapshot BoundTextLastTick;

	/** Was the editable text showing a password last Tick() (allows a forcible text layout update when changing state) */
	bool bWasPasswordLastTick;

	/** The text that appears when there is no text in the text box */
	TAttribute<FText> HintText;

	/** The text to be searched for */
	TAttribute<FText> BoundSearchText;

	/** The state of BoundSearchText last Tick() (used to allow updates when the text is changed) */
	FTextSnapshot BoundSearchTextLastTick;

	/** The active search text (set from BeginSearch) */
	FText SearchText;

	/** The case-sensitivity of the active search (set from BeginSearch) */
	ESearchCase::Type SearchCase;

	/** The map to look up the index of each search result (key is the starting location of each matched string)*/
	TMap<FTextLocation, int32> SearchResultToIndexMap;

	/** The active search result index */ 
	int32 CurrentSearchResultIndex;

	/** Whether text wraps onto a new line when it's length exceeds this width; if this value is zero or negative, no wrapping occurs. */
	TAttribute<float> WrapTextAt;

	/** True if we're wrapping text automatically based on the computed horizontal space for this widget */
	TAttribute<bool> AutoWrapText;

	/** The wrapping policy we're using */
	TAttribute<ETextWrappingPolicy> WrappingPolicy;

	/** The amount of blank space left around the edges of text area */
	TAttribute<FMargin> Margin;

	/** How the text should be aligned with the margin */
	TAttribute<ETextJustify::Type> Justification;

	/** The amount to scale each lines height by */
	TAttribute<float> LineHeightPercentage;

	/** Whether to leave extra space below the last line due to line height */
	TAttribute<bool> ApplyLineHeightToBottomLine;

	/** The information used to help identify who owns this text layout in the case of an error */
	TAttribute<FString> DebugSourceInfo;

	/** Virtual keyboard handler for this text layout */
	TSharedPtr<FVirtualKeyboardEntry> VirtualKeyboardEntry;

	/** True if the IME context for this text layout has been registered with the input method manager */
	bool bHasRegisteredTextInputMethodContext;

	/** IME context for this text layout */
	TSharedPtr<FTextInputMethodContext> TextInputMethodContext;

	/** Notification interface object for IMEs */
	TSharedPtr<ITextInputMethodChangeNotifier> TextInputMethodChangeNotifier;

	/** Layout highlighter used to draw the cursor */
	TSharedPtr<SlateEditableTextTypes::FCursorLineHighlighter> CursorLineHighlighter;

	/** Layout highlighter used to draw an active text composition */
	TSharedPtr<SlateEditableTextTypes::FTextCompositionHighlighter> TextCompositionHighlighter;

	/** Layout highlighter used to draw the active text selection */
	TSharedPtr<SlateEditableTextTypes::FTextSelectionHighlighter> TextSelectionHighlighter;

	/** Layout highlighter used to draw the active search selection */
	TSharedPtr<SlateEditableTextTypes::FTextSearchHighlighter> SearchSelectionHighlighter;

	/** Line highlights that have been added from this editable text layout (used for cleanup without removing) */
	TArray<FTextLineHighlight> ActiveLineHighlights;

	/** The scroll offset (in unscaled Slate units) for this text */
	FVector2f ScrollOffset;

	/** If set, the pending data containing a position that should be scrolled into view */
	TOptional<SlateEditableTextTypes::FScrollInfo> PositionToScrollIntoView;

	/** That start of the selection when there is a selection. The end is implicitly wherever the cursor happens to be. */
	TOptional<FTextLocation> SelectionStart;

	/** The user probably wants the cursor where they last explicitly positioned it horizontally. */
	float PreferredCursorScreenOffsetInLine;

	/** Current cursor data */
	SlateEditableTextTypes::FCursorInfo CursorInfo;

	/** Undo states */
	TArray<SlateEditableTextTypes::FUndoState> UndoStates;

	/** Current undo state level that we've rolled back to, or INDEX_NONE if we haven't undone. Used for 'Redo'. */
	int32 CurrentUndoLevel;

	/** Undo state that will be pushed if text is actually changed between calls to BeginEditTransation() and EndEditTransaction() */
	TOptional<SlateEditableTextTypes::FUndoState> StateBeforeChangingText;

	/** Track the number transactions that are opened */
	int32 NumTransactionsOpened;

	/** Original text undo state */
	SlateEditableTextTypes::FUndoState OriginalText;

	/** True if we're currently selecting text by dragging the mouse cursor with the left button held down */
	bool bIsDragSelecting;

	/** True if the last mouse down caused us to receive keyboard focus */
	bool bWasFocusedByLastMouseDown;

	/** True if characters were selected by dragging since the last keyboard focus.  Used for text selection. */
	bool bHasDragSelectedSinceFocused;

	/** Whether the text has been changed by a virtual keyboard */
	bool bTextChangedByVirtualKeyboard;

	/** Whether the text has been committed by a virtual keyboard */
	bool bTextCommittedByVirtualKeyboard;

	/** What text was submitted by a virtual keyboard */
	FText VirtualKeyboardText;

	/** How the text was committed by the virtual keyboard */
	ETextCommit::Type VirtualKeyboardTextCommitType;

	/** Override for the overflow policy. If this is not set the text style setting is used */
	TOptional<ETextOverflowPolicy> OverflowPolicyOverride;

	/** The last known size of the widget from the previous OnPaint, used to recalculate wrapping */
	FVector2f CachedSize;

	/** A list commands to execute if a user presses the corresponding key-binding in the text box */
	TSharedPtr<FUICommandList> UICommandList;

	/** Information about any active context menu widgets */
	FChunrealActiveTextEditContextMenu ActiveContextMenu;

	/** Whether the cursor position has been changed externally*/
	bool bSelectionChangedExternally;

	/** The boundaries of the external selection */
	int ExternalSelectionStart;
	int ExternalSelectionEnd;

};
