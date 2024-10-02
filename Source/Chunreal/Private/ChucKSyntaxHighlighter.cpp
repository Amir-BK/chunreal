// Fill out your copyright notice in the Description page of Project Settings.


#include "ChucKSyntaxHighlighter.h"

namespace ChucKSyntax

{
	const TCHAR* ChuckKeywords[] =
	{
		TEXT("if"),
		TEXT("else"),
		TEXT("while"),
		TEXT("until"),
		TEXT("for"),
		TEXT("repeat"),
		TEXT("break"),
		TEXT("continue"),
		TEXT("return"),
		TEXT("switch"),
		TEXT("class"),
		TEXT("extends"),
		TEXT("public"),
		TEXT("static"),
		TEXT("pure"),
		TEXT("this"),
		TEXT("super"),
		TEXT("interface"),
		TEXT("implements"),
		TEXT("protected"),
		TEXT("private"),
		TEXT("function"),
		TEXT("fun"),
		TEXT("spork"),
		TEXT("new"),
		TEXT("const"),
		TEXT("global"),
		TEXT("now"),
		TEXT("true"),
		TEXT("false"),
		TEXT("maybe"),
		TEXT("null"),
		TEXT("NULL"),
		TEXT("me"),
		TEXT("pi"),
		TEXT("samp"),
		TEXT("ms"),
		TEXT("second"),
		TEXT("minute"),
		TEXT("hour"),
		TEXT("day"),
		TEXT("week"),
		TEXT("eon"),
		TEXT("dac"),
		TEXT("adc"),
		TEXT("blackhole"),
		TEXT("bunghole"),
		//types

		TEXT("int"),
		TEXT("float"),
		TEXT("time"),
		TEXT("dur"),
		TEXT("void"),
		TEXT("vec3"),
		TEXT("vec4"),
		TEXT("complex"),
		TEXT("polar"),
		TEXT("string"),
		TEXT("Event")


	};

	const TCHAR* ChuckOperators[] =
	{
		TEXT("/*"),
		TEXT("*/"),
		TEXT("//"),
		TEXT("\""),
		TEXT("\'"),
		TEXT("::"),
		TEXT(":"),
		TEXT("+="),
		TEXT("++"),
		TEXT("+"),
		TEXT("--"),
		TEXT("-="),
		TEXT("-"),
		TEXT("("),
		TEXT(")"),
		TEXT("["),
		TEXT("]"),
		TEXT("."),
		TEXT("->"),
		TEXT("!="),
		TEXT("!"),
		TEXT("&="),
		TEXT("~"),
		TEXT("&"),
		TEXT("*="),
		TEXT("*"),
		TEXT("->"),
		TEXT("/="),
		TEXT("/"),
		TEXT("%="),
		TEXT("%"),
		TEXT("<<="),
		TEXT("<<"),
		TEXT("<="),
		TEXT("<"),
		TEXT(">>="),
		TEXT(">>"),
		TEXT(">="),
		TEXT(">"),
		TEXT("=="),
		TEXT("&&"),
		TEXT("&"),
		TEXT("^="),
		TEXT("^"),
		TEXT("|="),
		TEXT("||"),
		TEXT("|"),
		TEXT("?"),
		TEXT("="),

		//chuck operators
		TEXT("++"),
		TEXT("--"),
		TEXT(":"),
		TEXT("+"),
		TEXT("-"),
		TEXT("*"),
		TEXT("/"),
		TEXT("%"),
		TEXT("::"),
		TEXT("=="),
		TEXT("!="),
		TEXT("<"),
		TEXT(">"),
		TEXT("<="),
		TEXT(">="),
		TEXT("&&"),
		TEXT("||"),
		TEXT("&"),
		TEXT("|"),
		TEXT("^"),
		TEXT(">>"),
		TEXT("<<"),
		TEXT("="),
		TEXT("?"),
		TEXT("!"),
		TEXT("~"),
		TEXT("<<<"),
		TEXT(">>>"),
		TEXT("=>"),
		TEXT("!=>"),
		TEXT("=^"),
		TEXT("=v"),
		TEXT("@=>"),
		TEXT("+=>"),
		TEXT("-=>"),
		TEXT("*=>"),
		TEXT("/=>"),
		TEXT("&=>"),
		TEXT("|=>"),
		TEXT("^=>"),
		TEXT(">>=>"),
		TEXT("<<=>"),
		TEXT("%=>"),
		TEXT("@"),
		TEXT("@@"),
		TEXT("->"),
		TEXT("<-"),
		TEXT(".")

	};

}


class FChucKSyntaxTokenizer : public ISyntaxTokenizer
{

	TArray<FString> Keywords;
	TArray<FString> Operators;
public:

	static TSharedRef<FChucKSyntaxTokenizer> Create()
	{
		return MakeShareable(new FChucKSyntaxTokenizer());
	}

	virtual ~FChucKSyntaxTokenizer()
	{

	}
protected:

	virtual void Process(TArray<FTokenizedLine>& OutTokenizedLines, const FString& Input) override
	{
		TArray<FTextRange> LineRanges;
		FTextRange::CalculateLineRangesFromString(Input, LineRanges);
		TokenizeLineRanges(Input, LineRanges, OutTokenizedLines);
	};

	FChucKSyntaxTokenizer()
	{
		for (const auto& Keyword : ChucKSyntax::ChuckKeywords)
		{
			Keywords.Add(Keyword);
		}

		for (const auto& Operator : ChucKSyntax::ChuckOperators)
		{
			Operators.Add(Operator);
		}

	}

	static FORCEINLINE bool IsAlpha(TCHAR Char)
	{
		return (Char >= 'a' && Char <= 'z') || (Char >= 'A' && Char <= 'Z');
	}

	static FORCEINLINE bool IsDigit(TCHAR Char)
	{
		return Char >= '0' && Char <= '9';
	}

	static FORCEINLINE bool IsAlphaOrDigit(TCHAR Char)
	{
		return IsAlpha(Char) || IsDigit(Char);
	}

	void TokenizeLineRanges(const FString& Input, const TArray<FTextRange>& LineRanges, TArray<FTokenizedLine>& OutTokenizedLines)
	{
		// Tokenize line ranges
		for (const FTextRange& LineRange : LineRanges)
		{
			FTokenizedLine TokenizedLine;
			TokenizedLine.Range = LineRange;

			if (TokenizedLine.Range.IsEmpty())
			{
				TokenizedLine.Tokens.Emplace(FToken(ETokenType::Literal, TokenizedLine.Range));
			}
			else
			{
				int32 CurrentOffset = LineRange.BeginIndex;

				while (CurrentOffset < LineRange.EndIndex)
				{
					const TCHAR* CurrentString = &Input[CurrentOffset];
					const TCHAR CurrentChar = Input[CurrentOffset];

					//if current char is "." print something, we'll figure it out later
					if (CurrentChar == TEXT('.'))
					{
						//UE_LOG(LogTemp, Warning, TEXT("Current char is a dot"));
						//continue;
					}

					bool bHasMatchedSyntax = false;

					// Greedy matching for operators
					for (const FString& Operator : Operators)
					{
						if (FCString::Strncmp(CurrentString, *Operator, Operator.Len()) == 0)
						{
							const int32 SyntaxTokenEnd = CurrentOffset + Operator.Len();
							TokenizedLine.Tokens.Emplace(FToken(ETokenType::Syntax, FTextRange(CurrentOffset, SyntaxTokenEnd)));

							check(SyntaxTokenEnd <= LineRange.EndIndex);

							bHasMatchedSyntax = true;
							//UE_CLOG(bHasMatchedSyntax, LogTemp, Warning, TEXT("Matched Operator: %s"), *Operator);
							CurrentOffset = SyntaxTokenEnd;
							break;
						}
					}

					if (bHasMatchedSyntax)
					{
						continue;
					}

					int32 PeekOffset = CurrentOffset + 1;
					if (CurrentChar == TEXT('#'))
					{
						// Match PreProcessorKeywords
						// They only contain letters
						while (PeekOffset < LineRange.EndIndex)
						{
							const TCHAR PeekChar = Input[PeekOffset];


							if (!IsAlpha(PeekChar))
							{
								break;
							}

							PeekOffset++;
						}
					}
					else if (IsAlpha(CurrentChar))
					{
						// Match Identifiers,
						// They start with a letter and contain
						// letters or numbers
						while (PeekOffset < LineRange.EndIndex)
						{
							const TCHAR PeekChar = Input[PeekOffset];

							if (!IsAlphaOrDigit(PeekChar))
							{
								break;
							}

							PeekOffset++;
						}
					}

					
					const int32 CurrentStringLength = PeekOffset - CurrentOffset;

					// Check if it is an reserved keyword
					for (const FString& Keyword : Keywords)
					{
						if (FCString::Strncmp(CurrentString, *Keyword, CurrentStringLength) == 0)
						{
							const int32 SyntaxTokenEnd = CurrentOffset + CurrentStringLength;
							TokenizedLine.Tokens.Emplace(FToken(ETokenType::Syntax, FTextRange(CurrentOffset, SyntaxTokenEnd)));

							check(SyntaxTokenEnd <= LineRange.EndIndex);

							bHasMatchedSyntax = true;
							CurrentOffset = SyntaxTokenEnd;
							break;
						}
					}

					if (bHasMatchedSyntax)
					{
						continue;
					}

					// If none matched, consume the character(s) as text
					const int32 TextTokenEnd = CurrentOffset + CurrentStringLength;
					TokenizedLine.Tokens.Emplace(FToken(ETokenType::Literal, FTextRange(CurrentOffset, TextTokenEnd)));
					CurrentOffset = TextTokenEnd;
				}
			}

			OutTokenizedLines.Add(TokenizedLine);
		}
	};

};

class FWhiteSpaceTextRun : public FSlateTextRun
{
public:
	static TSharedRef<FWhiteSpaceTextRun> Create(
		const FRunInfo& InRunInfo,
		const TSharedRef<const FString>& InText,
		const FTextBlockStyle& Style,
		const FTextRange& InRange,
		int32 NumSpacesPerTab)
	{
		return MakeShareable(new FWhiteSpaceTextRun(InRunInfo, InText, Style, InRange, NumSpacesPerTab));
	}

public:
	virtual FVector2D Measure(
		int32 StartIndex,
		int32 EndIndex,
		float Scale,
		const FRunTextContext& TextContext
	) const override
	{
		const FVector2D ShadowOffsetToApply((EndIndex == Range.EndIndex) ? FMath::Abs(Style.ShadowOffset.X * Scale) : 0.0f, FMath::Abs(Style.ShadowOffset.Y * Scale));

		if (EndIndex - StartIndex == 0)
		{
			return FVector2D(ShadowOffsetToApply.X * Scale, GetMaxHeight(Scale));
		}

		// count tabs
		int32 TabCount = 0;
		for (int32 Index = StartIndex; Index < EndIndex; Index++)
		{
			if ((*Text)[Index] == TEXT('\t'))
			{
				TabCount++;
			}
		}

		const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		FVector2D Size = FontMeasure->Measure(**Text, StartIndex, EndIndex, Style.Font, true, Scale) + ShadowOffsetToApply;

		Size.X -= TabWidth * TabCount * Scale;
		Size.X += SpaceWidth * TabCount * NumSpacesPerTab * Scale;

		return Size;
	}

protected:
	FWhiteSpaceTextRun(
		const FRunInfo& InRunInfo,
		const TSharedRef<const FString>& InText,
		const FTextBlockStyle& InStyle,
		const FTextRange& InRange,
		int32 InNumSpacesPerTab) :
		FSlateTextRun(InRunInfo, InText, InStyle, InRange),
		NumSpacesPerTab(InNumSpacesPerTab)
	{
		// measure tab width
		const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		TabWidth = FontMeasure->Measure(TEXT("\t"), 0, 1, Style.Font, true, 1.0f).X;
		SpaceWidth = FontMeasure->Measure(TEXT(" "), 0, 1, Style.Font, true, 1.0f).X;
	}

private:
	int32 NumSpacesPerTab;

	double TabWidth;

	double SpaceWidth;
};




inline TSharedRef<FChucKSyntaxHighlighterMarshaller> FChucKSyntaxHighlighterMarshaller::Create(const FSyntaxTextStyle& InSyntaxTextStyle)
{
	return MakeShareable(new FChucKSyntaxHighlighterMarshaller(CreateTokenizer(), InSyntaxTextStyle));

}

inline TSharedPtr<ISyntaxTokenizer> FChucKSyntaxHighlighterMarshaller::CreateTokenizer()
{
	return FChucKSyntaxTokenizer::Create();
}

inline void FChucKSyntaxHighlighterMarshaller::ParseTokens(const FString& SourceString, FTextLayout& TargetTextLayout, TArray<ISyntaxTokenizer::FTokenizedLine> TokenizedLines)
{
	TArray<FTextLayout::FNewLineData> LinesToAdd;
	LinesToAdd.Reserve(TokenizedLines.Num());

	// Parse the tokens, generating the styled runs for each line
	int32 LineNo = 0;
	EParseState ParseState = EParseState::None;

	for (auto& TokenizedLine : TokenizedLines)
	{
		LinesToAdd.Add(ProcessTokenizedLine(TokenizedLine, LineNo, SourceString, ParseState));
		LineNo++;
	}

	TargetTextLayout.AddLines(LinesToAdd);

	//yntaxHighlighterTextLayoutMarshaller::ParseTokens(SourceString, TargetTextLayout, TokenizedLines);
}

inline FTextLayout::FNewLineData FChucKSyntaxHighlighterMarshaller::ProcessTokenizedLine(const ISyntaxTokenizer::FTokenizedLine& TokenizedLine, const int32& LineNumber, const FString& SourceString, EParseState& ParseState)
{
	TSharedRef<FString> ModelString = MakeShareable(new FString());
	TArray< TSharedRef< IRun > > Runs;

	for (const ISyntaxTokenizer::FToken& Token : TokenizedLine.Tokens)
	{
		const FString TokenText = SourceString.Mid(Token.Range.BeginIndex, Token.Range.Len());

		const FTextRange ModelRange(ModelString->Len(), ModelString->Len() + TokenText.Len());
		ModelString->Append(TokenText);

		FRunInfo RunInfo(TEXT("SyntaxHighlight.HLSL.Normal"));
		FTextBlockStyle TextBlockStyle = SyntaxTextStyle.NormalTextStyle;

		const bool bIsWhitespace = FString(TokenText).TrimEnd().IsEmpty();
		if (!bIsWhitespace)
		{
			bool bHasMatchedSyntax = false;
			if (Token.Type == ISyntaxTokenizer::ETokenType::Syntax)
			{
				if (ParseState == EParseState::None && TokenText == TEXT("\""))
				{
					RunInfo.Name = TEXT("SyntaxHighlight.HLSL.String");
					TextBlockStyle = SyntaxTextStyle.StringTextStyle;
					ParseState = EParseState::LookingForString;
					bHasMatchedSyntax = true;
				}
				else if (ParseState == EParseState::LookingForString && TokenText == TEXT("\""))
				{
					RunInfo.Name = TEXT("SyntaxHighlight.HLSL.Normal");
					TextBlockStyle = SyntaxTextStyle.NormalTextStyle;
					ParseState = EParseState::None;
				}
				else if (ParseState == EParseState::None && TokenText == TEXT("\'"))
				{
					RunInfo.Name = TEXT("SyntaxHighlight.HLSL.String");
					TextBlockStyle = SyntaxTextStyle.StringTextStyle;
					ParseState = EParseState::LookingForCharacter;
					bHasMatchedSyntax = true;
				}
				else if (ParseState == EParseState::LookingForCharacter && TokenText == TEXT("\'"))
				{
					RunInfo.Name = TEXT("SyntaxHighlight.HLSL.Normal");
					TextBlockStyle = SyntaxTextStyle.NormalTextStyle;
					ParseState = EParseState::None;
				}
				else if (ParseState == EParseState::None && TokenText.StartsWith(TEXT("#")))
				{
					RunInfo.Name = TEXT("SyntaxHighlight.HLSL.PreProcessorKeyword");
					TextBlockStyle = SyntaxTextStyle.PreProcessorKeywordTextStyle;
					ParseState = EParseState::None;
				}
				else if (ParseState == EParseState::None && TokenText == TEXT("//"))
				{
					RunInfo.Name = TEXT("SyntaxHighlight.HLSL.Comment");
					TextBlockStyle = SyntaxTextStyle.CommentTextStyle;
					ParseState = EParseState::LookingForSingleLineComment;
				}
				else if (ParseState == EParseState::None && TokenText == TEXT("/*"))
				{
					RunInfo.Name = TEXT("SyntaxHighlight.HLSL.Comment");
					TextBlockStyle = SyntaxTextStyle.CommentTextStyle;
					ParseState = EParseState::LookingForMultiLineComment;
				}
				else if (ParseState == EParseState::LookingForMultiLineComment && TokenText == TEXT("*/"))
				{
					RunInfo.Name = TEXT("SyntaxHighlight.HLSL.Comment");
					TextBlockStyle = SyntaxTextStyle.CommentTextStyle;
					ParseState = EParseState::None;
				}
				else if (ParseState == EParseState::None && TChar<WIDECHAR>::IsAlpha(TokenText[0]))
				{
					RunInfo.Name = TEXT("SyntaxHighlight.HLSL.Keyword");
					TextBlockStyle = SyntaxTextStyle.KeywordTextStyle;
					ParseState = EParseState::None;
				}
				else if (ParseState == EParseState::None && !TChar<WIDECHAR>::IsAlpha(TokenText[0]))
				{
					RunInfo.Name = TEXT("SyntaxHighlight.HLSL.Operator");
					TextBlockStyle = SyntaxTextStyle.OperatorTextStyle;
					ParseState = EParseState::None;
				}
			}

			// It's possible that we fail to match a syntax token if we're in a state where it isn't parsed
			// In this case, we treat it as a literal token
			if (Token.Type == ISyntaxTokenizer::ETokenType::Literal || !bHasMatchedSyntax)
			{
				if (ParseState == EParseState::LookingForString)
				{
					RunInfo.Name = TEXT("SyntaxHighlight.HLSL.String");
					TextBlockStyle = SyntaxTextStyle.StringTextStyle;
				}
				else if (ParseState == EParseState::LookingForCharacter)
				{
					RunInfo.Name = TEXT("SyntaxHighlight.HLSL.String");
					TextBlockStyle = SyntaxTextStyle.StringTextStyle;
				}
				else if (ParseState == EParseState::LookingForSingleLineComment)
				{
					RunInfo.Name = TEXT("SyntaxHighlight.HLSL.Comment");
					TextBlockStyle = SyntaxTextStyle.CommentTextStyle;
				}
				else if (ParseState == EParseState::LookingForMultiLineComment)
				{
					RunInfo.Name = TEXT("SyntaxHighlight.HLSL.Comment");
					TextBlockStyle = SyntaxTextStyle.CommentTextStyle;
				}
			}

			TSharedRef< ISlateRun > Run = FSlateTextRun::Create(RunInfo, ModelString, TextBlockStyle, ModelRange);
			Runs.Add(Run);
		}
		else
		{
			RunInfo.Name = TEXT("SyntaxHighlight.HLSL.WhiteSpace");
			TSharedRef< ISlateRun > Run = FWhiteSpaceTextRun::Create(RunInfo, ModelString, TextBlockStyle, ModelRange, 4);
			Runs.Add(Run);
		}
	}

	if (ParseState != EParseState::LookingForMultiLineComment)
	{
		ParseState = EParseState::None;
	}
	return FTextLayout::FNewLineData(MoveTemp(ModelString), MoveTemp(Runs));

}

FChucKSyntaxHighlighterMarshaller::FChucKSyntaxHighlighterMarshaller(TSharedPtr<ISyntaxTokenizer> InTokenizer, const FSyntaxTextStyle& InSyntaxTextStyle)
	:	FSyntaxHighlighterTextLayoutMarshaller(MoveTemp(InTokenizer))
	, SyntaxTextStyle(InSyntaxTextStyle)
{
}
