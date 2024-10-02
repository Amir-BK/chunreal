// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Attribute.h"
#include "SlateGlobals.h"
#include "Layout/Margin.h"
#include "Fonts/SlateFontInfo.h"
#include "Input/CursorReply.h"
#include "Input/Reply.h"
#include "Layout/Visibility.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Styling/SlateTypes.h"
#include "Styling/CoreStyle.h"
#include "Framework/Text/IRun.h"
#include "Framework/Text/TextLayout.h"
#include "Widgets/SWidget.h"
#include "Widgets/Layout/SScrollBar.h"
#include "Framework/SlateDelegates.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#if WITH_FANCY_TEXT
	#include "Widgets/Text/ISlateEditableTextWidget.h"
	#include "Framework/Text/SlateTextLayoutFactory.h"
#endif
#include <ChunrealSlateEditableTextLayout.h>

class FActiveTimerHandle;
class FArrangedChildren;
class FPaintArgs;
class FSlateEditableTextLayout;
class FSlateWindowElementList;
class ITextLayoutMarshaller;
enum class ETextShapingMethod : uint8;

#if WITH_FANCY_TEXT

class ITextLayoutMarshaller;
class FSlateEditableTextLayout;

/** An editable text widget that supports multiple lines and soft word-wrapping. */
class SChunrealMultiLineEditableText : public SWidget, public IChunrealSlateEditableTextWidget
{
public:

	/** Used to merge multiple text edit transactions within a scope */
	struct FScopedEditableTextTransaction
	{
	public:
		FScopedEditableTextTransaction(TSharedPtr<SChunrealMultiLineEditableText> InText)
			: Text(InText)
		{
			Text->BeginEditTransaction();
		}

		~FScopedEditableTextTransaction()
		{
			Text->EndEditTransaction();	
		};

	private:
		TSharedPtr<SChunrealMultiLineEditableText> Text;
	};
	
	/** Called when the cursor is moved within the text area */
	DECLARE_DELEGATE_OneParam( FOnCursorMoved, const FTextLocation& );

	SLATE_BEGIN_ARGS( SChunrealMultiLineEditableText )
		: _Text()
		, _HintText()
		, _SearchText()
		, _Marshaller()
		, _WrapTextAt( 0.0f )
		, _AutoWrapText(false)
		, _WrappingPolicy(ETextWrappingPolicy::DefaultWrapping)
		, _TextStyle( &FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>( "NormalText" ) )
		, _Font()
		, _Margin( FMargin() )
		, _LineHeightPercentage( 1.0f )
		, _ApplyLineHeightToBottomLine( true )
		, _Justification( ETextJustify::Left )
		, _IsReadOnly(false)
		, _OnTextChanged()
		, _OnTextCommitted()
		, _AllowMultiLine(true)
		, _SelectAllTextWhenFocused(false)
		, _SelectWordOnMouseDoubleClick(true)
		, _ClearTextSelectionOnFocusLoss(true)
		, _RevertTextOnEscape(false)
		, _ClearKeyboardFocusOnCommit(true)
		, _AllowContextMenu(true)
		, _OnCursorMoved()
		, _ContextMenuExtender()
		, _ModiferKeyForNewLine(EModifierKey::None)
		, _VirtualKeyboardOptions( FVirtualKeyboardOptions() )
		, _VirtualKeyboardTrigger(EChunrealVirtualKeyboardTrigger::OnFocusByPointer)
		, _VirtualKeyboardDismissAction(EChunrealVirtualKeyboardDismissAction::TextChangeOnDismiss)
		, _TextShapingMethod()
		, _TextFlowDirection()
		, _OverflowPolicy()
	{
		_Clipping = EWidgetClipping::ClipToBounds;
	}

		/** The initial text that will appear in the widget. */
		SLATE_ATTRIBUTE(FText, Text)

		/** Hint text that appears when there is no text in the text box */
		SLATE_ATTRIBUTE(FText, HintText)

		/** Text to search for (a new search is triggered whenever this text changes) */
		SLATE_ATTRIBUTE(FText, SearchText)

		/** The marshaller used to get/set the raw text to/from the text layout. */
		SLATE_ARGUMENT(TSharedPtr< ITextLayoutMarshaller >, Marshaller)

		/** Whether text wraps onto a new line when it's length exceeds this width; if this value is zero or negative, no wrapping occurs. */
		SLATE_ATTRIBUTE(float, WrapTextAt)

		/** Whether to wrap text automatically based on the widget's computed horizontal space.  IMPORTANT: Using automatic wrapping can result
		    in visual artifacts, as the the wrapped size will computed be at least one frame late!  Consider using WrapTextAt instead.  The initial 
			desired size will not be clamped.  This works best in cases where the text block's size is not affecting other widget's layout. */
		SLATE_ATTRIBUTE(bool, AutoWrapText)

		/** The wrapping policy to use */
		SLATE_ATTRIBUTE(ETextWrappingPolicy, WrappingPolicy)

		/** Pointer to a style of the text block, which dictates the font, color, and shadow options. */
		SLATE_STYLE_ARGUMENT(FTextBlockStyle, TextStyle)

		/** Font color and opacity (overrides Style) */
		SLATE_ATTRIBUTE(FSlateFontInfo, Font)

		/** The amount of blank space left around the edges of text area. */
		SLATE_ATTRIBUTE(FMargin, Margin)

		/** The amount to scale each lines height by. */
		SLATE_ATTRIBUTE(float, LineHeightPercentage)

		/** The amount to scale each lines height by. */
		SLATE_ATTRIBUTE(bool, ApplyLineHeightToBottomLine)

		/** How the text should be aligned with the margin. */
		SLATE_ATTRIBUTE(ETextJustify::Type, Justification)

		/** Sets whether this text box can actually be modified interactively by the user */
		SLATE_ATTRIBUTE(bool, IsReadOnly)

		/** The horizontal scroll bar widget */
		SLATE_ARGUMENT(TSharedPtr< SScrollBar >, HScrollBar)

		/** The vertical scroll bar widget */
		SLATE_ARGUMENT(TSharedPtr< SScrollBar >, VScrollBar)

		/**
		 * This is NOT for validating input!
		 * 
		 * Called whenever a character is typed.
		 * Not called for copy, paste, or any other text changes!
		 */
		SLATE_EVENT(FOnIsTypedCharValid, OnIsTypedCharValid)

		/** Called whenever the text is changed programmatically or interactively by the user */
		SLATE_EVENT(FOnTextChanged, OnTextChanged)

		/** Called whenever the text is committed.  This happens when the user presses enter or the text box loses focus. */
		SLATE_EVENT(FOnTextCommitted, OnTextCommitted)

		/** Whether to allow multi-line text */
		SLATE_ATTRIBUTE(bool, AllowMultiLine)

		/** Whether to select all text when the user clicks to give focus on the widget */
		SLATE_ATTRIBUTE(bool, SelectAllTextWhenFocused)

		/** Whether to select word on mouse double click on the widget */
		SLATE_ATTRIBUTE(bool, SelectWordOnMouseDoubleClick)

		/** Whether to clear text selection when focus is lost */
		SLATE_ATTRIBUTE(bool, ClearTextSelectionOnFocusLoss)

		/** Whether to allow the user to back out of changes when they press the escape key */
		SLATE_ATTRIBUTE(bool, RevertTextOnEscape)

		/** Whether to clear keyboard focus when pressing enter to commit changes */
		SLATE_ATTRIBUTE(bool, ClearKeyboardFocusOnCommit)

		/** Whether to prevent the context menu from being displayed  */
		SLATE_ATTRIBUTE(bool, AllowContextMenu)

		/** Delegate to call before a context menu is opened. User returns the menu content or null to the disable context menu */
		SLATE_EVENT(FOnContextMenuOpening, OnContextMenuOpening)

		/** Called whenever the horizontal scrollbar is moved by the user */
		SLATE_EVENT(FOnUserScrolled, OnHScrollBarUserScrolled)

		/** Called whenever the vertical scrollbar is moved by the user */
		SLATE_EVENT(FOnUserScrolled, OnVScrollBarUserScrolled)

		/** Called when the cursor is moved within the text area */
		SLATE_EVENT(FOnCursorMoved, OnCursorMoved)

		/** Callback delegate to have first chance handling of the OnKeyChar event */
		SLATE_EVENT(FOnKeyChar, OnKeyCharHandler)

		/** Callback delegate to have first chance handling of the OnKeyDown event */
		SLATE_EVENT(FOnKeyDown, OnKeyDownHandler)

		/** Menu extender for the right-click context menu */
		SLATE_EVENT(FMenuExtensionDelegate, ContextMenuExtender)

		/** Delegate used to create text layouts for this widget. If none is provided then FSlateTextLayout will be used. */
		SLATE_EVENT(FCreateChunrealSlateTextLayout, CreateSlateTextLayout)

		/** The optional modifier key necessary to create a newline when typing into the editor. */
		SLATE_ARGUMENT(EModifierKey::Type, ModiferKeyForNewLine)

		/** Additional options for the virtual keyboard used by this widget */
		SLATE_ARGUMENT(FVirtualKeyboardOptions, VirtualKeyboardOptions)

		/** The type of event that will trigger the display of the virtual keyboard */
		SLATE_ATTRIBUTE(EChunrealVirtualKeyboardTrigger, VirtualKeyboardTrigger)

		/** The message action to take when the virtual keyboard is dismissed by the user */
		SLATE_ATTRIBUTE(EChunrealVirtualKeyboardDismissAction, VirtualKeyboardDismissAction)

		/** Which text shaping method should we use? (unset to use the default returned by GetDefaultTextShapingMethod) */
		SLATE_ARGUMENT( TOptional<ETextShapingMethod>, TextShapingMethod )
		
		/** Which text flow direction should we use? (unset to use the default returned by GetDefaultTextFlowDirection) */
		SLATE_ARGUMENT( TOptional<ETextFlowDirection>, TextFlowDirection )

		/** Determines what happens to text that is clipped and doesnt fit within the clip rect for this widget */
		SLATE_ARGUMENT(TOptional<ETextOverflowPolicy>, OverflowPolicy)

	SLATE_END_ARGS()

	CHUNREAL_API SChunrealMultiLineEditableText();
	CHUNREAL_API ~SChunrealMultiLineEditableText();

	CHUNREAL_API void Construct( const FArguments& InArgs );

	/**
	 * Sets the text for this text block
	 */
	CHUNREAL_API void SetText(const TAttribute< FText >& InText);

	/**
	 * Returns the text string
	 *
	 * @return  Text string
	 */
	CHUNREAL_API FText GetText() const;

	/**
	 * Returns the plain text string without richtext formatting
	 * @return  Text string
	 */
	CHUNREAL_API FText GetPlainText() const;

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
	
	/**
	 * Sets the text that appears when there is no text in the text box
	 */
	CHUNREAL_API void SetHintText(const TAttribute< FText >& InHintText);

	/** Get the text that appears when there is no text in the text box */
	CHUNREAL_API FText GetHintText() const;

	/** Set the text that is currently being searched for (if any) */
	CHUNREAL_API void SetSearchText(const TAttribute<FText>& InSearchText);

	/** Get the text that is currently being searched for (if any) */
	CHUNREAL_API FText GetSearchText() const;

	/** Get the index of the search result (0 if none) */
	CHUNREAL_API int32 GetSearchResultIndex() const;

	/** Get the total number of search results (0 if none) */
	CHUNREAL_API int32 GetNumSearchResults() const;

	/** See attribute TextStyle */
	CHUNREAL_API void SetTextStyle(const FTextBlockStyle* InTextStyle);

	/** See attribute Font */
	CHUNREAL_API void SetFont(const TAttribute< FSlateFontInfo >& InNewFont);
	CHUNREAL_API FSlateFontInfo GetFont() const;

	/** See TextShapingMethod attribute */
	CHUNREAL_API void SetTextShapingMethod(const TOptional<ETextShapingMethod>& InTextShapingMethod);

	/** See TextFlowDirection attribute */
	CHUNREAL_API void SetTextFlowDirection(const TOptional<ETextFlowDirection>& InTextFlowDirection);

	/** See WrapTextAt attribute */
	CHUNREAL_API void SetWrapTextAt(const TAttribute<float>& InWrapTextAt);

	/** See AutoWrapText attribute */
	CHUNREAL_API void SetAutoWrapText(const TAttribute<bool>& InAutoWrapText);

	/** Set WrappingPolicy attribute */
	CHUNREAL_API void SetWrappingPolicy(const TAttribute<ETextWrappingPolicy>& InWrappingPolicy);

	/** See LineHeightPercentage attribute */
	CHUNREAL_API void SetLineHeightPercentage(const TAttribute<float>& InLineHeightPercentage);

	/** See ApplyLineHeightToBottomLine attribute */
	CHUNREAL_API void SetApplyLineHeightToBottomLine(const TAttribute<bool>& InApplyLineHeightToBottomLine);

	/** See Margin attribute */
	CHUNREAL_API void SetMargin(const TAttribute<FMargin>& InMargin);

	/** See Justification attribute */
	CHUNREAL_API void SetJustification(const TAttribute<ETextJustify::Type>& InJustification);

	/** Sets the overflow policy for this text block */
	CHUNREAL_API void SetOverflowPolicy(TOptional<ETextOverflowPolicy> InOverflowPolicy);

	/** See the AllowContextMenu attribute */
	CHUNREAL_API void SetAllowContextMenu(const TAttribute< bool >& InAllowContextMenu);

	/** Set the VirtualKeyboardDismissAction attribute */
	CHUNREAL_API void SetVirtualKeyboardDismissAction(TAttribute< EChunrealVirtualKeyboardDismissAction > InVirtualKeyboardDismissAction);

	/**
	 * Sets whether to select word on the mouse double click
	 *
	 * @param  InSelectWordOnMouseDoubleClick		Select word on the mouse double click
	 */
	CHUNREAL_API void SetSelectWordOnMouseDoubleClick(const TAttribute<bool>& InSelectWordOnMouseDoubleClick);
	
	/** Sets the ReadOnly attribute */
	CHUNREAL_API void SetIsReadOnly(const TAttribute< bool >& InIsReadOnly);

	/** Get the number of Text Lines */
	CHUNREAL_API int32 GetTextLineCount();

	/**
	 * Sets whether to select all text when the user clicks to give focus on the widget
	 *
	 * @param  InSelectAllTextWhenFocused	Select all text when the user clicks?
	 */
	CHUNREAL_API void SetSelectAllTextWhenFocused(const TAttribute<bool>& InSelectAllTextWhenFocused);

	/**
	 * Sets whether to clear text selection when focus is lost
	 *
	 * @param  InClearTextSelectionOnFocusLoss	Clear text selection when focus is lost?
	 */
	CHUNREAL_API void SetClearTextSelectionOnFocusLoss(const TAttribute<bool>& InClearTextSelectionOnFocusLoss);

	/**
	 * Sets whether to allow the user to back out of changes when they press the escape key
	 *
	 * @param  InRevertTextOnEscape			Allow the user to back out of changes?
	 */
	CHUNREAL_API void SetRevertTextOnEscape(const TAttribute<bool>& InRevertTextOnEscape);

	/**
	 * Sets whether to clear keyboard focus when pressing enter to commit changes
	 *
	 * @param  InClearKeyboardFocusOnCommit		Clear keyboard focus when pressing enter?
	 */
	CHUNREAL_API void SetClearKeyboardFocusOnCommit(const TAttribute<bool>& InClearKeyboardFocusOnCommit);

	/** Query to see if any text is selected within the document */
	CHUNREAL_API bool AnyTextSelected() const;

	/** Select all the text in the document */
	CHUNREAL_API void SelectAllText();

	/** Select a block of text */
	CHUNREAL_API void SelectText(const FTextLocation& InSelectionStart, const FTextLocation& InCursorLocation);
	
	/** Clear the active text selection */
	CHUNREAL_API void ClearSelection();

	/** Get the currently selected text */
	CHUNREAL_API FText GetSelectedText() const;

	/** Get the current selection */
	CHUNREAL_API FTextSelection GetSelection() const;

	/** Delete any currently selected text */
	CHUNREAL_API void DeleteSelectedText();

	/** Insert the given text at the current cursor position, correctly taking into account new line characters */
	CHUNREAL_API void InsertTextAtCursor(const FText& InText);
	CHUNREAL_API void InsertTextAtCursor(const FString& InString);

	/** Insert the given run at the current cursor position */
	CHUNREAL_API void InsertRunAtCursor(TSharedRef<IRun> InRun);

	/** Move the cursor to the given location in the document (will also scroll to this point) */
	CHUNREAL_API void GoTo(const FTextLocation& NewLocation);

	/** Move the cursor specified location */
	CHUNREAL_API void GoTo(const EChunrealTextLocation NewLocation);

	/** Scroll to the given location in the document (without moving the cursor) */
	CHUNREAL_API void ScrollTo(const FTextLocation& NewLocation);

	/** Scroll to the given location in the document (without moving the cursor) */
	CHUNREAL_API void ScrollTo(const EChunrealTextLocation NewLocation);

	/** Apply the given style to the currently selected text (or insert a new run at the current cursor position if no text is selected) */
	CHUNREAL_API void ApplyToSelection(const FRunInfo& InRunInfo, const FTextBlockStyle& InStyle);

	/** Begin a new text search (this is called automatically when the bound search text changes) */
	CHUNREAL_API void BeginSearch(const FText& InSearchText, const ESearchCase::Type InSearchCase = ESearchCase::IgnoreCase, const bool InReverse = false);

	/** Advance the current search to the next match (does nothing if not currently searching) */
	CHUNREAL_API void AdvanceSearch(const bool InReverse = false);

	/** Get the run currently under the cursor, or null if there is no run currently under the cursor */
	CHUNREAL_API TSharedPtr<const IRun> GetRunUnderCursor() const;

	/** Get the runs currently that are current selected, some of which may be only partially selected */
	CHUNREAL_API TArray<TSharedRef<const IRun>> GetSelectedRuns() const;

	/** Get the interaction position of the cursor (where to insert, delete, etc, text from/to) */
	CHUNREAL_API FTextLocation GetCursorLocation() const;

	/** Get the character at Location */
	CHUNREAL_API TCHAR GetCharacterAt(const FTextLocation& Location) const;

	/** Get the horizontal scroll bar widget */
	CHUNREAL_API TSharedPtr<const SScrollBar> GetHScrollBar() const;

	/** Get the vertical scroll bar widget */
	CHUNREAL_API TSharedPtr<const SScrollBar> GetVScrollBar() const;

	/** Refresh this editable text immediately, rather than wait for the usual caching mechanisms to take affect on the text Tick */
	CHUNREAL_API void Refresh();

	/**
	 * Sets the OnKeyCharHandler to provide first chance handling of the OnKeyChar event
	 *
	 * @param InOnKeyCharHandler			Delegate to call during OnKeyChar event
	 */
	void SetOnKeyCharHandler(FOnKeyChar InOnKeyCharHandler)
	{
		OnKeyCharHandler = InOnKeyCharHandler;
	}

	/**
	 * Sets the OnKeyDownHandler to provide first chance handling of the OnKeyDown event
	 *
	 * @param InOnKeyDownHandler			Delegate to call during OnKeyDown event
	 */
	void SetOnKeyDownHandler(FOnKeyDown InOnKeyDownHandler)
	{
		OnKeyDownHandler = InOnKeyDownHandler;
	}

	/**
	 *	Force a single scroll operation. 
	 */
	CHUNREAL_API void ForceScroll(int32 UserIndex, float ScrollAxisMagnitude);

protected:
	//~ Begin SWidget Interface
	CHUNREAL_API virtual void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime ) override;
	CHUNREAL_API virtual int32 OnPaint( const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled ) const override;
	CHUNREAL_API virtual void CacheDesiredSize(float LayoutScaleMultiplier) override;
	CHUNREAL_API virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	CHUNREAL_API virtual FChildren* GetChildren() override;
	CHUNREAL_API virtual void OnArrangeChildren( const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren ) const override;
	CHUNREAL_API virtual bool SupportsKeyboardFocus() const override;
	CHUNREAL_API virtual FReply OnKeyChar( const FGeometry& MyGeometry,const FCharacterEvent& InCharacterEvent ) override;
	CHUNREAL_API virtual FReply OnKeyDown( const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent ) override;
	CHUNREAL_API virtual FReply OnKeyUp( const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent ) override;
	CHUNREAL_API virtual FReply OnFocusReceived( const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent ) override;
	CHUNREAL_API virtual void OnFocusLost( const FFocusEvent& InFocusEvent ) override;
	CHUNREAL_API virtual FReply OnMouseButtonDown( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent ) override;
	CHUNREAL_API virtual FReply OnMouseButtonUp( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent ) override;
	CHUNREAL_API virtual FReply OnMouseMove( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent ) override;
	CHUNREAL_API virtual FReply OnMouseWheel( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent ) override;
	CHUNREAL_API virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	CHUNREAL_API virtual FCursorReply OnCursorQuery( const FGeometry& MyGeometry, const FPointerEvent& CursorEvent ) const override;
	CHUNREAL_API virtual bool IsInteractable() const override;
	CHUNREAL_API virtual bool ComputeVolatility() const override;
	//~ End SWidget Interface

protected:
	CHUNREAL_API void OnHScrollBarMoved(const float InScrollOffsetFraction);
	CHUNREAL_API void OnVScrollBarMoved(const float InScrollOffsetFraction);

	/** Return whether a RMB+Drag scroll operation is taking place */
	CHUNREAL_API bool IsRightClickScrolling() const;

public:
	//~ Begin ISlateEditableTextWidget Interface
	CHUNREAL_API virtual bool IsTextReadOnly() const override;
	CHUNREAL_API virtual bool IsTextPassword() const override;
	CHUNREAL_API virtual bool IsMultiLineTextEdit() const override;
	//~ End ISlateEditableTextWidget Interface

protected:
	//~ Begin ISlateEditableTextWidget Interface
	CHUNREAL_API virtual bool ShouldJumpCursorToEndWhenFocused() const override;
	CHUNREAL_API virtual bool ShouldSelectAllTextWhenFocused() const override;
	CHUNREAL_API virtual bool ShouldClearTextSelectionOnFocusLoss() const override;
	CHUNREAL_API virtual bool ShouldRevertTextOnEscape() const override;
	CHUNREAL_API virtual bool ShouldClearKeyboardFocusOnCommit() const override;
	CHUNREAL_API virtual bool ShouldSelectAllTextOnCommit() const override;
	CHUNREAL_API virtual bool ShouldSelectWordOnMouseDoubleClick() const override;
	CHUNREAL_API virtual bool CanInsertCarriageReturn() const override;
	CHUNREAL_API virtual bool CanTypeCharacter(const TCHAR InChar) const override;
	CHUNREAL_API virtual void EnsureActiveTick() override;
	CHUNREAL_API virtual EKeyboardType GetVirtualKeyboardType() const override;
	CHUNREAL_API virtual FVirtualKeyboardOptions GetVirtualKeyboardOptions() const override;
	CHUNREAL_API virtual EChunrealVirtualKeyboardTrigger GetVirtualKeyboardTrigger() const override;
	CHUNREAL_API virtual EChunrealVirtualKeyboardDismissAction GetVirtualKeyboardDismissAction() const override;
	CHUNREAL_API virtual TSharedRef<SWidget> GetSlateWidget() override;
	CHUNREAL_API virtual TSharedPtr<SWidget> GetSlateWidgetPtr() override;
	CHUNREAL_API virtual TSharedPtr<SWidget> BuildContextMenuContent() const override;
	CHUNREAL_API virtual void OnTextChanged(const FText& InText) override;
	CHUNREAL_API virtual void OnTextCommitted(const FText& InText, const ETextCommit::Type InTextAction) override;
	CHUNREAL_API virtual void OnCursorMoved(const FTextLocation& InLocation) override;
	CHUNREAL_API virtual float UpdateAndClampHorizontalScrollBar(const float InViewOffset, const float InViewFraction, const EVisibility InVisiblityOverride) override;
	CHUNREAL_API virtual float UpdateAndClampVerticalScrollBar(const float InViewOffset, const float InViewFraction, const EVisibility InVisiblityOverride) override;
	//~ End ISlateEditableTextWidget Interface

protected:
	/** Called to begin an undoable editable text transaction, marked as protected for use with FScopedEditableTextTransaction only */
	CHUNREAL_API void BeginEditTransaction();

	/** Called to end an undoable editable text transaction, marked as protected for use with FScopedEditableTextTransaction only */
	CHUNREAL_API void EndEditTransaction();

protected:
	/** The text layout that deals with the editable text */
	TUniquePtr<FChunrealSlateEditableTextLayout> EditableTextLayout;

	/** Whether to allow multi-line text */
	TAttribute<bool> bAllowMultiLine;

	/** Whether to select all text when the user clicks to give focus on the widget */
	TAttribute<bool> bSelectAllTextWhenFocused;

	/** Whether to clear text selection when focus is lost */
	TAttribute<bool> bClearTextSelectionOnFocusLoss;

	/** Whether to select work on mouse double click */
	TAttribute<bool> bSelectWordOnMouseDoubleClick;

	/** True if any changes should be reverted if we receive an escape key */
	TAttribute<bool> bRevertTextOnEscape;

	/** True if we want the text control to lose focus on an text commit/revert events */
	TAttribute<bool> bClearKeyboardFocusOnCommit;

	/** Whether the context menu can be opened */
	TAttribute<bool> bAllowContextMenu;

	/** Sets whether this text box can actually be modified interactively by the user */
	TAttribute<bool> bIsReadOnly;

	/** Delegate to call before a context menu is opened */
	FOnContextMenuOpening OnContextMenuOpening;

	/** Called when a character is typed and we want to know if the text field supports typing this character. */
	FOnIsTypedCharValid OnIsTypedCharValid;

	/** Called whenever the text is changed programmatically or interactively by the user */
	FOnTextChanged OnTextChangedCallback;

	/** Called whenever the text is committed.  This happens when the user presses enter or the text box loses focus. */
	FOnTextCommitted OnTextCommittedCallback;

	/** Called when the cursor is moved within the text area */
	FOnCursorMoved OnCursorMovedCallback;

	/** The horizontal scroll bar widget */
	TSharedPtr<SScrollBar> HScrollBar;

	/** The vertical scroll bar widget */
	TSharedPtr<SScrollBar> VScrollBar;

	/** Called whenever the horizontal scrollbar is moved by the user */
	FOnUserScrolled OnHScrollBarUserScrolled;

	/** Called whenever the vertical scrollbar is moved by the user */
	FOnUserScrolled OnVScrollBarUserScrolled;

	/** Menu extender for right-click context menu */
	TSharedPtr<FExtender> MenuExtender;

	/** The optional modifier key necessary to create a newline when typing into the editor. */
	EModifierKey::Type ModiferKeyForNewLine;

	/** The timer that is actively driving this widget to Tick() even when Slate is idle */
	TWeakPtr<FActiveTimerHandle> ActiveTickTimer;

	/** How much we scrolled while RMB was being held */
	float AmountScrolledWhileRightMouseDown;

	/** Whether a software cursor is currently active */
	bool bIsSoftwareCursor;

	/**	The current position of the software cursor */
	FVector2f SoftwareCursorPosition;

	/** Callback delegate to have first chance handling of the OnKeyChar event */
	FOnKeyChar OnKeyCharHandler;

	/** Callback delegate to have first chance handling of the OnKeyDown event */
	FOnKeyDown OnKeyDownHandler;

	/** Options to use for the virtual keyboard summoned by this widget */
	FVirtualKeyboardOptions VirtualKeyboardOptions;

	/** The type of event that will trigger the display of the virtual keyboard */
	TAttribute<EChunrealVirtualKeyboardTrigger> VirtualKeyboardTrigger;

	/** The message action to take when the virtual keyboard is dismissed by the user */
	TAttribute<EChunrealVirtualKeyboardDismissAction> VirtualKeyboardDismissAction;
};

#endif //WITH_FANCY_TEXT
