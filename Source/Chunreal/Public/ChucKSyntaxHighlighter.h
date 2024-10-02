// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Text/SyntaxHighlighterTextLayoutMarshaller.h"
#include "Framework/Text/SyntaxTokenizer.h"
#include "Framework/Text/TextLayout.h"
#include "HAL/Platform.h"
#include "Styling/SlateTypes.h"
#include "Templates/SharedPointer.h"
#include "Framework/Text/IRun.h"
#include "Framework/Text/TextLayout.h"
#include "Framework/Text/ISlateRun.h"
#include "Framework/Text/SlateTextRun.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "ChucKSyntaxHighlighter.generated.h"


/**
 * 
 */
UCLASS()
class CHUNREAL_API UChucKSyntaxHighlighter : public UObject
{
	GENERATED_BODY()
	
};

class CHUNREAL_API FChucKSyntaxHighlighterMarshaller : public FSyntaxHighlighterTextLayoutMarshaller
{
	struct FSyntaxTextStyle
	{
		FSyntaxTextStyle(
			const FTextBlockStyle& InNormalTextStyle,
			const FTextBlockStyle& InOperatorTextStyle,
			const FTextBlockStyle& InKeywordTextStyle,
			const FTextBlockStyle& InStringTextStyle,
			const FTextBlockStyle& InNumberTextStyle,
			const FTextBlockStyle& InCommentTextStyle,
			const FTextBlockStyle& InPreProcessorKeywordTextStyle,
			const FTextBlockStyle& InErrorTextStyle
		) :
			NormalTextStyle(InNormalTextStyle),
			OperatorTextStyle(InOperatorTextStyle),
			KeywordTextStyle(InKeywordTextStyle),
			StringTextStyle(InStringTextStyle),
			NumberTextStyle(InNumberTextStyle),
			CommentTextStyle(InCommentTextStyle),
			PreProcessorKeywordTextStyle(InPreProcessorKeywordTextStyle),
			ErrorTextStyle(InErrorTextStyle)
		{
		}

		FTextBlockStyle NormalTextStyle;
		FTextBlockStyle OperatorTextStyle;
		FTextBlockStyle KeywordTextStyle;
		FTextBlockStyle StringTextStyle;
		FTextBlockStyle NumberTextStyle;
		FTextBlockStyle CommentTextStyle;
		FTextBlockStyle PreProcessorKeywordTextStyle;
		FTextBlockStyle ErrorTextStyle;
	};

	enum class EParseState : uint8
	{
		None,
		LookingForString,
		LookingForCharacter,
		LookingForSingleLineComment,
		LookingForMultiLineComment,
	};

	static FSyntaxTextStyle GetSyntaxTextStyle()
	{
		//FAppStyle::GetWidgetStyle<FTextBlockStyle>("SyntaxHighlight.HLSL.Normal");
		FTextBlockStyle NormalTextStyle = FTextBlockStyle();
		NormalTextStyle.SetColorAndOpacity(FLinearColor::White);
		NormalTextStyle.SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 10));
		NormalTextStyle.SetShadowOffset(FVector2D::ZeroVector);
		NormalTextStyle.SetShadowColorAndOpacity(FLinearColor::Black);
		NormalTextStyle.SetHighlightColor(FLinearColor::White);

		FTextBlockStyle OperatorTextStyle = FTextBlockStyle(NormalTextStyle);
		OperatorTextStyle.SetColorAndOpacity(FLinearColor::Red);


		//make styles for the rest of the syntax
		FTextBlockStyle KeywordTextStyle = FTextBlockStyle(NormalTextStyle);
		KeywordTextStyle.SetColorAndOpacity(FLinearColor(FColor(0xff006ab4)));
		//make keyword style bold
		KeywordTextStyle.SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 10));

		FTextBlockStyle StringTextStyle = FTextBlockStyle(NormalTextStyle);
		StringTextStyle.SetColorAndOpacity(FLinearColor(FColor(0xffdfd706)));

		FTextBlockStyle NumberTextStyle = FTextBlockStyle(NormalTextStyle);
		NumberTextStyle.SetColorAndOpacity(FLinearColor(FColor(0xff6db3a8)));

		FTextBlockStyle CommentTextStyle = FTextBlockStyle(NormalTextStyle);
		CommentTextStyle.SetColorAndOpacity(FLinearColor(FColor(0xff57a64a)));

		FTextBlockStyle PreProcessorKeywordTextStyle = FTextBlockStyle(NormalTextStyle);
		PreProcessorKeywordTextStyle.SetColorAndOpacity(FLinearColor(FColor(0xffcfcfcf)));

		FTextBlockStyle ErrorTextStyle = FTextBlockStyle(NormalTextStyle);
		ErrorTextStyle.SetColorAndOpacity(FLinearColor::Red);



		return FSyntaxTextStyle(NormalTextStyle,
			OperatorTextStyle,
			KeywordTextStyle,
			StringTextStyle,
			NumberTextStyle,
			CommentTextStyle,
			PreProcessorKeywordTextStyle,
			ErrorTextStyle);
			
	}

public:

	static TSharedRef<FChucKSyntaxHighlighterMarshaller> Create(const FSyntaxTextStyle& InSyntaxTextStyle = GetSyntaxTextStyle());

	static TSharedPtr<ISyntaxTokenizer> CreateTokenizer();
protected:

	virtual void ParseTokens(const FString& SourceString, FTextLayout& TargetTextLayout, TArray<ISyntaxTokenizer::FTokenizedLine> TokenizedLines) override;

	virtual FTextLayout::FNewLineData ProcessTokenizedLine(const ISyntaxTokenizer::FTokenizedLine& TokenizedLine, const int32& LineNumber, const FString& SourceString, EParseState& CurrentParseState);;
	
	FChucKSyntaxHighlighterMarshaller(TSharedPtr<ISyntaxTokenizer> InTokenizer, const FSyntaxTextStyle& InSyntaxTextStyle);



	/** Styles used to display the text */
	FSyntaxTextStyle SyntaxTextStyle;

};
