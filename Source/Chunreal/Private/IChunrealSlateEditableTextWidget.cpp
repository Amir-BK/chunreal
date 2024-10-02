// Copyright Epic Games, Inc. All Rights Reserved.

#include "IChunrealSlateEditableTextWidget.h"

FChunrealMoveCursor FChunrealMoveCursor::Cardinal(EChunrealCursorMoveGranularity Granularity, FIntPoint Direction, EChunrealCursorAction Action)
{
	return FChunrealMoveCursor(Granularity, EChunrealCursorMoveMethod::Cardinal, Direction, 1.0f, Action);
}

FChunrealMoveCursor FChunrealMoveCursor::ViaScreenPointer(FVector2D LocalPosition, float GeometryScale, EChunrealCursorAction Action)
{
	return FChunrealMoveCursor(EChunrealCursorMoveGranularity::Character, EChunrealCursorMoveMethod::ScreenPosition, LocalPosition, GeometryScale, Action);
}

EChunrealCursorMoveMethod FChunrealMoveCursor::GetMoveMethod() const
{
	return Method;
}

bool FChunrealMoveCursor::IsVerticalMovement() const
{
	return DirectionOrPosition.Y != 0.0f;
}

bool FChunrealMoveCursor::IsHorizontalMovement() const
{
	return DirectionOrPosition.X != 0.0f;
}

FIntPoint FChunrealMoveCursor::GetMoveDirection() const
{
	return FIntPoint(DirectionOrPosition.X, DirectionOrPosition.Y);
}

EChunrealCursorAction FChunrealMoveCursor::GetAction() const
{
	return Action;
}

FVector2D FChunrealMoveCursor::GetLocalPosition() const
{
	return FVector2D(DirectionOrPosition);
}

EChunrealCursorMoveGranularity FChunrealMoveCursor::GetGranularity() const
{
	return Granularity;
}

float FChunrealMoveCursor::GetGeometryScale() const
{
	return GeometryScale;
}

FChunrealMoveCursor::FChunrealMoveCursor(EChunrealCursorMoveGranularity InGranularity, EChunrealCursorMoveMethod InMethod, FVector2D InDirectionOrPosition, float InGeometryScale, EChunrealCursorAction InAction)
	: Granularity(InGranularity)
	, Method(InMethod)
	, DirectionOrPosition(InDirectionOrPosition)
	, Action(InAction)
	, GeometryScale(InGeometryScale)
{
	// Movement actions are assumed to be exclusively vertical or exclusively horizontal
	// There is no great reason to assume this, but a lot of the code was written to deal with
	// events, which must be one or the other.
	ensure(Method == EChunrealCursorMoveMethod::ScreenPosition || (Method == EChunrealCursorMoveMethod::Cardinal && IsVerticalMovement() != IsHorizontalMovement()));
}
