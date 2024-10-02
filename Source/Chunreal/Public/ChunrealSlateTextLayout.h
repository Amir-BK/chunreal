// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Misc/Attribute.h"
#include "Styling/SlateTypes.h"
#include "Framework/Text/IRun.h"
#include "Framework/Text/TextLayout.h"
#include "Widgets/SWidget.h"
#include "Layout/Children.h"

class FArrangedChildren;
class FPaintArgs;
class FSlateWindowElementList;

class FChunrealSlateTextLayout : public FTextLayout
{
public:

	static CHUNREAL_API TSharedRef< FChunrealSlateTextLayout > Create(SWidget* InOwner, FTextBlockStyle InDefaultTextStyle);

	CHUNREAL_API FChildren* GetChildren();

	CHUNREAL_API virtual void ArrangeChildren( const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren ) const;

	CHUNREAL_API virtual int32 OnPaint( const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled ) const;

	CHUNREAL_API virtual void EndLayout() override;

	CHUNREAL_API void SetDefaultTextStyle(FTextBlockStyle InDefaultTextStyle);
	CHUNREAL_API const FTextBlockStyle& GetDefaultTextStyle() const;

	CHUNREAL_API void SetIsPassword(const TAttribute<bool>& InIsPassword);

protected:

	CHUNREAL_API FChunrealSlateTextLayout(SWidget* InOwner, FTextBlockStyle InDefaultTextStyle);

	CHUNREAL_API virtual int32 OnPaintHighlights(const FPaintArgs& Args, const FTextLayout::FLineView& LineView, const TArray<FLineViewHighlight>& Highlights, const FTextBlockStyle& DefaultTextStyle, const FGeometry& AllottedGeometry, const FSlateRect& ClippingRect, FSlateWindowElementList& OutDrawElements, const int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const;

	CHUNREAL_API virtual void AggregateChildren();

	CHUNREAL_API virtual TSharedRef<IRun> CreateDefaultTextRun(const TSharedRef<FString>& NewText, const FTextRange& NewRange) const override;

protected:
	/** Default style used by the TextLayout */
	FTextBlockStyle DefaultTextStyle;

private:

	TSlotlessChildren< SWidget > Children;

	/** This this layout displaying a password? */
	TAttribute<bool> bIsPassword;

	friend class FChunrealSlateTextLayoutFactory;
};
