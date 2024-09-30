
#pragma once

#include "DSP/Dsp.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Sound/SoundEffectSubmix.h"

#include "ChuckEffectsClasses.generated.h"

USTRUCT(BlueprintType)
struct FSubmixChuckEffectSettings
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Initialization, meta = (ClampMin = "10.0", UIMin = "10.0", UIMax = "20000.0"))
	float MaximumDelayLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Realtime, meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "20000.0"))
	float InterpolationTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Realtime, meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "20000.0"))
	float DelayLength;

	FSubmixChuckEffectSettings()
		: MaximumDelayLength(2000.0f)
		, InterpolationTime(400.0f)
		, DelayLength(1000.0f)
	{
	}
};


class CHUNREAL_API FSubmixChuckEffect : public FSoundEffectSubmix
{

	//~ Begin FSoundEffectSubmix
	virtual void Init(const FSoundEffectSubmixInitData& InData) override
	{

	};
	virtual void OnProcessAudio(const FSoundEffectSubmixInputData& InData, FSoundEffectSubmixOutputData& OutData) override
	{

	};
	//~ End FSoundEffectSubmix

	
};

UCLASS()
class CHUNREAL_API USubmixChuckEffectPreset : public USoundEffectSubmixPreset
{
	GENERATED_BODY()
	public:
	EFFECT_PRESET_METHODS(SubmixChuckEffect)

		virtual void OnInit() override {};

	UFUNCTION(BlueprintCallable, Category = "Audio|Effects|Delay")
	void SetDefaultSettings(const FSubmixChuckEffectSettings& InSettings)
	{
		FScopeLock ScopeLock(&SettingsCritSect);
		SettingsCopy = InSettings;
		Update();
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetDefaultSettings, Category = SubmixEffectPreset, Meta = (ShowOnlyInnerProperties))
	FSubmixChuckEffectSettings Settings;


};