
#pragma once

#include "DSP/Dsp.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Sound/SoundEffectSubmix.h"
#include "Sound/SoundEffectSource.h"
#include "ChuckInstance.h"
#include "Chunreal.h"
//#include "Chunreal/chuck/chuck.h"

#include "ChuckEffectsClasses.generated.h"

#pragma region ChuckSubmixEffect

USTRUCT(BlueprintType)
struct FSubmixChuckEffectSettings
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Realtime)
	TMap<FName, FAudioParameter > Params;


	//chuck ref
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Realtime)
	TObjectPtr<UChuckProcessor> ChuckInstance;

};


class CHUNREAL_API FSubmixChuckEffect : public FSoundEffectSubmix
{
	~FSubmixChuckEffect()
	{
		DeleteOurChuck();

	}

	//~ Begin FSoundEffectSubmix
	virtual void Init(const FSoundEffectSubmixInitData& InData) override
	{
		SampleRate = InData.SampleRate;
	};
	virtual void OnProcessAudio(const FSoundEffectSubmixInputData& InData, FSoundEffectSubmixOutputData& OutData) override
	{
		//check for updates 
		OnPresetChanged();
		
		FChunrealModule::RunChuck(ChuckRef, InData.AudioBuffer->GetData(), OutData.AudioBuffer->GetData(), InData.NumFrames);
	};
	//~ End FSoundEffectSubmix

	virtual void OnPresetChanged() override;

	void DeleteOurChuck()
	{
		if (ChuckRef)
		{
			delete ChuckRef;
			ChuckRef = nullptr;
			bHasSporkedOnce = false;
		};
	}

	bool bHasSporkedOnce = false;


	FGuid CurrentChuckGuid;
	int32 SampleRate;
	int32 NumChannels = 2;
	ChucK* ChuckRef = nullptr;
	TObjectPtr<UChuckProcessor> ChuckProcessor; // should really make this name consistent with the rest of the codebase
	
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
	TMap<FName, FAudioParameter > Params;


	//chuck ref
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Realtime)
	TObjectPtr<UChuckProcessor> ChuckInstance;


};

class CHUNREAL_API FSourceEffectChuck : public FSoundEffectSource
{

	~FSourceEffectChuck()
	{
		DeleteOurChuck();

	}

	virtual void Init(const FSoundEffectSourceInitData& InitData) override
	{
		SampleRate = InitData.SampleRate;
	};

	virtual void OnPresetChanged() override;

	virtual void ProcessAudio(const FSoundEffectSourceInputData& InData, float* OutAudioBufferData) override
	{

		//check for updates 
		OnPresetChanged();

		//Process samples by ChucK
		FChunrealModule::RunChuck(ChuckRef, InData.InputSourceEffectBufferPtr, OutAudioBufferData, InData.NumSamples / NumChannels);
	};

	void DeleteOurChuck()
	{
		if (ChuckRef)
		{
			delete ChuckRef;
			ChuckRef = nullptr;
			bHasSporkedOnce = false;
		}


	}


	bool bHasSporkedOnce = false;


	FGuid CurrentChuckGuid;
	int32 SampleRate;
	int32 NumChannels = 2;
	ChucK* ChuckRef = nullptr;
	TObjectPtr<UChuckProcessor> ChuckProcessor; // should really make this name consistent with the rest of the codebase
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