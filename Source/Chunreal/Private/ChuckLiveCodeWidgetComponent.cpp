// Fill out your copyright notice in the Description page of Project Settings.


#include "ChuckLiveCodeWidgetComponent.h"

void UChuckLiveCodeWidgetComponent::InitWidget()
{
	DrawSize = Size;
	WidgetClass = CodeEditorWidgetClass;
	Super::InitWidget();

	//GetWidget()->SetDesiredSizeInViewport(Size);
	ChuckCodeWidget = Cast<UChuckCodeEditorWidget>(GetUserWidgetObject());
	if (ChuckCodeWidget)
	{
	//	ChuckCodeWidget->SetChuckCode(ChuckCode);
		ChuckCodeWidget->SetBoxSize(Size);
		ChuckCodeWidget->OnChuckWidgetUnfocus.AddUniqueDynamic(this, &UChuckLiveCodeWidgetComponent::ChuckWidgetUnfocus);
		if (ChuckCode)
		{
			ChuckCodeWidget->SetCode(ChuckCode);
		}

	}


}

UChuckLiveCodeWidgetComponent::UChuckLiveCodeWidgetComponent()
{
	if (CodeEditorWidgetClass)
	{
		WidgetClass = CodeEditorWidgetClass;
	}
	else
	{
		WidgetClass = UChuckCodeEditorWidget::StaticClass();
	}
	bDrawAtDesiredSize = false;
}
