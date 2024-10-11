// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "ChuckInstance.h"
#include "ChuckCodeEditorWidget.h"
#include "ChuckLiveCodeWidgetComponent.generated.h"



/**
 * 
 */
UCLASS(ClassGroup = (Chunreal), meta = (BlueprintSpawnableComponent), BlueprintType, HideCategories = (UserInterface), Blueprintable, ComponentWrapperClass)
class CHUNREAL_API UChuckLiveCodeWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()



protected:
	//the object that will be used to compile and run the chuck code, will be automatically created if not provided
	UPROPERTY(EditAnywhere, Category = "Chuck", BlueprintSetter = "SetChuckCode")
	TObjectPtr<UChuckCode> ChuckCode;

	//widget desired size
	UPROPERTY(EditAnywhere, Category = "Chuck")
	FIntPoint Size = FIntPoint(300, 500);

	//the instance of the chuck code widget
	UPROPERTY(Transient)
	TObjectPtr< UChuckCodeEditorWidget> ChuckCodeWidget;

public:

	UPROPERTY(BlueprintAssignable, Category = "Chuck")
	FOnChuckWidgetUnfocus OnChuckWidgetUnfocus;

	UFUNCTION(BlueprintCallable)
	void ChuckWidgetCommitCode()
	{
		UE_LOG(LogTemp, Log, TEXT("ChuckWidgetCommitCode"));
		if (ChuckCodeWidget)
		{
			//ChuckCodeWidget->CommitCode();
		}
	}

	UFUNCTION()
	void ChuckWidgetUnfocus()
	{
		UE_LOG(LogTemp, Log, TEXT("ChuckWidgetUnfocus"));
		OnChuckWidgetUnfocus.Broadcast();
	}

	UFUNCTION(BlueprintSetter)
	void SetChuckCode(UChuckCode* InChuckCode)
	{
		ChuckCode = InChuckCode;
	}

	//allows overriding the default code editor widget class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChucK", meta = (ExposeOnSpawn = "true"))
	TSubclassOf<UChuckCodeEditorWidget> CodeEditorWidgetClass;

	virtual void InitWidget() override;
	
	UChuckLiveCodeWidgetComponent();

};
