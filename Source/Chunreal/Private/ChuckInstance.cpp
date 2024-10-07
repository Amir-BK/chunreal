// Fill out your copyright notice in the Description page of Project Settings.


#include "ChuckInstance.h"
#include "Chunreal/chuck/chuck_globals.h"
#include "HAL/PlatformApplicationMisc.h"

// Inherited via IAudioProxyDataFactory

DEFINE_LOG_CATEGORY_STATIC(LogChuckInstance, VeryVerbose, All);

DEFINE_METASOUND_DATA_TYPE(Metasound::FChuckProcessor, "ChucK Processor")
DEFINE_METASOUND_DATA_TYPE(Metasound::FChuckInstance, "ChucK Instance")

namespace ChunrealEventRegistry
{
	static TMap<t_CKINT, TTuple<FString, FOnGlobalEventExecuted>> EventDelegates;
	//native delegates map
	static TMap<t_CKINT, TTuple<FString, FOnGlobalEventExecutedNative>> NativeEventDelegates;
	static int EventIdCounter = 0;
}

inline TSharedPtr<Audio::IProxyData> UChuckCode::CreateProxyData(const Audio::FProxyDataInitParams& InitParams)
{

	return MakeShared<FChuckCodeProxy>(this);
}


ChucK* UChuckCode::CreateChuckVm(int32 InNumChannels)
{
	ChucK* theChuck = new ChucK();
	const auto PlatformAudioSettings = FAudioPlatformSettings::GetPlatformSettings(FPlatformProperties::GetRuntimeSettingsClassName());
	const auto PlatformSampleRate = PlatformAudioSettings.SampleRate;
	
	//UE_LOG(LogChucKMidiNode, VeryVerbose, TEXT("Creating new chuck for asset: %s"));

	theChuck = new ChucK();
	theChuck->setLogLevel(2);
	//Initialize Chuck params
	theChuck->setParam(CHUCK_PARAM_SAMPLE_RATE, PlatformSampleRate);
	theChuck->setParam(CHUCK_PARAM_INPUT_CHANNELS, InNumChannels);
	theChuck->setParam(CHUCK_PARAM_OUTPUT_CHANNELS, InNumChannels);
	theChuck->setParam(CHUCK_PARAM_VM_ADAPTIVE, 0);
	theChuck->setParam(CHUCK_PARAM_VM_HALT, (t_CKINT)(false));
	//Chuck->setParam(CHUCK_PARAM_OTF_PORT, g_otf_port);
	//Chuck->setParam(CHUCK_PARAM_OTF_ENABLE, (t_CKINT)TRUE);
	//Chuck->setParam(CHUCK_PARAM_DUMP_INSTRUCTIONS, (t_CKINT)dump);
	theChuck->setParam(CHUCK_PARAM_AUTO_DEPEND, (t_CKINT)0);
	//Chuck->setParam(CHUCK_PARAM_DEPRECATE_LEVEL, deprecate_level);
	theChuck->setParam(CHUCK_PARAM_CHUGIN_ENABLE, true);
	//Chuck->setParam(CHUCK_PARAM_USER_CHUGINS, named_dls);
	//Chuck->setParam(CHUCK_PARAM_USER_CHUGIN_DIRECTORIES, dl_search_path);
	theChuck->setParam(CHUCK_PARAM_IS_REALTIME_AUDIO_HINT, true);

	//Set working directory
	FChunrealModule ChunrealModule = FModuleManager::Get().GetModuleChecked<FChunrealModule>("Chunreal");
	theChuck->setParam(CHUCK_PARAM_WORKING_DIRECTORY, TCHAR_TO_UTF8(*ChunrealModule.workingDirectory));


	return theChuck;
}


inline UChuckInstantiation::UChuckInstantiation()
{
	// Must be created by a valid ChuckCode object
	if (!IsTemplate()) {
		UE_LOG(LogTemp, Warning, TEXT("Chuck Instance Created"));
		ParentChuckCode = CastChecked<UChuckCode>(GetOuter());
		ParentChuckCode->OnChuckNeedsRecompile.AddUObject(this, &UChuckInstantiation::OnChuckCodeAssetChanged);
		CompileCode();
		//bAutoActivate = true;
	}


}

inline UChuckInstantiation::~UChuckInstantiation()
{
	if (ChuckVm)
	{
		delete ChuckVm;
	}

	if (IsValid(ParentChuckCode))
	{
		ParentChuckCode->OnChuckNeedsRecompile.RemoveAll(this);
	}

	UE_LOG(LogTemp, Warning, TEXT("Chuck Instance Destroyed"));

}

int UChuckInstantiation::SubscribeToGlobalEvent(FString EventName, const FOnGlobalEventExecuted& InDelegate) {
	// I think we don't need to check if is valid... we'll see
	t_CKINT EventID = ChunrealEventRegistry::EventIdCounter++;
	auto EventCallBack = [](t_CKINT inEventID) {
		//InDelegate.Execute(EventName);
		auto EventTuple = ChunrealEventRegistry::EventDelegates[inEventID];
		EventTuple.Value.ExecuteIfBound(EventTuple.Key);
		};

	const char* EventNameChar = TCHAR_TO_ANSI(*EventName);
	//add to global static map
	auto EventTuple = TTuple<FString, FOnGlobalEventExecuted>(EventName, InDelegate);
	ChunrealEventRegistry::EventDelegates.Add(TTuple<t_CKINT, TTuple<FString, FOnGlobalEventExecuted>>(EventID, EventTuple));

	ChuckVm->globals()->listenForGlobalEvent(EventNameChar, EventID, (EventCallBack), (t_CKBOOL)(true));

	return EventID;

}
int UChuckInstantiation::SubscribeToGlobalEventNative(FString EventName, const FOnGlobalEventExecutedNative& InDelegate)
{
	t_CKINT EventID = ChunrealEventRegistry::EventIdCounter++;
	auto EventCallBack = [](t_CKINT inEventID) {
		//InDelegate.Execute(EventName);
		auto EventTuple = ChunrealEventRegistry::NativeEventDelegates[inEventID];
		EventTuple.Value.ExecuteIfBound(EventTuple.Key);
		};

	const char* EventNameChar = TCHAR_TO_ANSI(*EventName);
	//add to global static map
	auto EventTuple = TTuple<FString, FOnGlobalEventExecutedNative>(EventName, InDelegate);
	ChunrealEventRegistry::NativeEventDelegates.Add(TTuple<t_CKINT, TTuple<FString, FOnGlobalEventExecutedNative>>(EventID, EventTuple));

	ChuckVm->globals()->listenForGlobalEvent(EventNameChar, EventID, (EventCallBack), (t_CKBOOL)(true));
	
	
	return EventID;
}
;

UChuckInstantiation* UChuckCode::SpawnChuckInstance()
{
	
	
	auto* NewChuck = NewObject<UChuckInstantiation>(this);
	
	
	return NewChuck;
}

void UChuckCode::CompileChuckAsset(ChucK* chuckRef)
{
	checkNoEntry();
	if (bIsAutoManaged)
	{
		FChunrealModule ChunrealModule = FModuleManager::Get().GetModuleChecked<FChunrealModule>("Chunreal");
		FString WorkingDir = ChunrealModule.workingDirectory;
		FChunrealModule::CompileChuckFile(chuckRef, TCHAR_TO_UTF8(*(SourcePath)));

	}
	else
	{
		FChunrealModule::CompileChuckCode(chuckRef, TCHAR_TO_UTF8(*Code));
	}
}

TSharedPtr<Audio::IProxyData> UChuckInstantiation::CreateProxyData(const Audio::FProxyDataInitParams& InitParams)
{
	return MakeShared<FChuckInstanceProxy>(this);
}

void UChuckSynthComponent::InitWithChuckInstance(UChuckInstantiation* InChuckInstance)
{

	ChuckVm = InChuckInstance->ChuckVm;

}