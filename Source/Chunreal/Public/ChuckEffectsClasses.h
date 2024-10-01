
#pragma once

#include "DSP/Dsp.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Sound/SoundEffectSubmix.h"
#include "Sound/SoundEffectSource.h"
#include "ChuckInstance.h"
#include "Chunreal.h"

#include "ChuckEffectsClasses.generated.h"

#pragma region ChuckSubmixEffect

USTRUCT(BlueprintType)
struct FSubmixChuckEffectSettings
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Realtime)
	TMap<FName, FAudioParameter > InitialParams;


	//chuck ref
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Realtime)
	TObjectPtr<UChuckProcessor> ChuckInstance;

	FSubmixChuckEffectSettings()

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

#pragma endregion

#pragma region ChuckSourceEffect

USTRUCT(BlueprintType)
struct CHUNREAL_API FSourceEffectChuckSettings
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Realtime)
	TMap<FName, FAudioParameter > InitialParams;


	//chuck ref
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Realtime)
	TObjectPtr<UChuckProcessor> ChuckInstance;

	FSourceEffectChuckSettings()
	{
	}
};

class CHUNREAL_API FSourceEffectChuck : public FSoundEffectSource
{
	virtual void Init(const FSoundEffectSourceInitData& InitData) override
	{

	};

	virtual void OnPresetChanged() override
	{

	};

	virtual void ProcessAudio(const FSoundEffectSourceInputData& InData, float* OutAudioBufferData) override
	{

	};
};

UCLASS(ClassGroup = AudioSourceEffect, meta = (BlueprintSpawnableComponent))
class CHUNREAL_API USourceEffectChuckPreset : public USoundEffectSourcePreset
{
	GENERATED_BODY()
public:
	EFFECT_PRESET_METHODS(SourceEffectChuck)

		virtual void OnInit() override {};

	UFUNCTION(BlueprintCallable, Category = "Audio|Effects|Delay")
	void SetDefaultSettings(const FSourceEffectChuckSettings& InSettings)
	{
		FScopeLock ScopeLock(&SettingsCritSect);
		SettingsCopy = InSettings;
		Update();
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetDefaultSettings, Category = SourceEffectPreset, Meta = (ShowOnlyInnerProperties))
	FSourceEffectChuckSettings Settings;

};

#pragma endregion