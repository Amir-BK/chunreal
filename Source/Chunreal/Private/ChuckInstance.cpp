// Fill out your copyright notice in the Description page of Project Settings.


#include "ChuckInstance.h"
#include "Chunreal/chuck/chuck_globals.h"
#include "HAL/PlatformApplicationMisc.h"


#define CHUGIN_PLATFORM_DIR TEXT("chugins-win64")

// Inherited via IAudioProxyDataFactory

static TAutoConsoleVariable<int32> ChuckLogLevel(TEXT("Chuck.LogLevel"), 2, TEXT("ChucK log level"), ECVF_Default);

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

TSharedPtr<Audio::IProxyData> UChuckCode::CreateProxyData(const Audio::FProxyDataInitParams& InitParams)
{

	return MakeShared<FChuckCodeProxy>(this);
}


ChucK* UChuckCode::CreateChuckVm(int32 InNumChannels)
{
	ChucK* theChuck = new ChucK();
	const auto PlatformAudioSettings = FAudioPlatformSettings::GetPlatformSettings(FPlatformProperties::GetRuntimeSettingsClassName());
	const auto PlatformSampleRate = PlatformAudioSettings.SampleRate;
	
	//UE_LOG(LogChucKMidiNode, VeryVerbose, TEXT("Creating new chuck for asset: %s"));
	FChunrealModule ChunrealModule = FModuleManager::Get().GetModuleChecked<FChunrealModule>("Chunreal");
	theChuck = new ChucK();
	theChuck->setLogLevel(ChuckLogLevel.GetValueOnAnyThread());
	//Initialize Chuck params
	theChuck->setParam(CHUCK_PARAM_USER_CHUGINS, TCHAR_TO_UTF8(*FPaths::Combine(*ChunrealModule.workingDirectory, TEXT("chugins"))));
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
	//Chuck->setParam(CHUCK_PARAM_CHUGIN_LIST_USER_DIR, TCHAR_TO_UTF8(*FString(ChunrealModule.workingDirectory + "/" + "chugins")));
	theChuck->setParam(CHUCK_PARAM_IS_REALTIME_AUDIO_HINT, true);

	//Set working directory

	theChuck->setParam(CHUCK_PARAM_WORKING_DIRECTORY, TCHAR_TO_UTF8(*ChunrealModule.workingDirectory));

	//get chugin directory param and print it
	//auto ChuginDir = theChuck->getParamString(CHUCK_PARAM_CHUGIN_DIRECTORY);
	//UE_LOG(LogChuckInstance, VeryVerbose, TEXT("Chugin Directory: %s"), UTF8_TO_TCHAR(ChuginDir.c_str()));

	//auto UserChuginDir = theChuck->getParamString(CHUCK_PARAM_CHUGIN_LIST_USER_DIR);
	//UE_LOG(LogChuckInstance, VeryVerbose, TEXT("User Chugin Directory: %s"), UTF8_TO_TCHAR(UserChuginDir.c_str()));

	//probe chugins
	//theChuck->probeChugins();


	return theChuck;
}



 UChuckInstantiation::UChuckInstantiation()
{
	// Must be created by a valid ChuckCode object
	if (!IsTemplate()) {
		UE_LOG(LogTemp, Warning, TEXT("Chuck Instance Created"));
		ParentChuckCode = CastChecked<UChuckCode>(GetOuter());
		ParentChuckCode->OnChuckNeedsRecompile.AddUObject(this, &UChuckInstantiation::OnChuckCodeAssetChanged);
		CompileCode();

	}


}

 UChuckInstantiation::~UChuckInstantiation()
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

//const char* EventNameChar = ;
	//add to global static map
	auto EventTuple = TTuple<FString, FOnGlobalEventExecuted>(EventName, InDelegate);
	ChunrealEventRegistry::EventDelegates.Add(TTuple<t_CKINT, TTuple<FString, FOnGlobalEventExecuted>>(EventID, EventTuple));

	ChuckVm->globals()->listenForGlobalEvent(TCHAR_TO_ANSI(*EventName), EventID, (EventCallBack), (t_CKBOOL)(true));

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

	//const char* EventNameChar = ;
	//add to global static map
	auto EventTuple = TTuple<FString, FOnGlobalEventExecutedNative>(EventName, InDelegate);
	ChunrealEventRegistry::NativeEventDelegates.Add(TTuple<t_CKINT, TTuple<FString, FOnGlobalEventExecutedNative>>(EventID, EventTuple));

	ChuckVm->globals()->listenForGlobalEvent(TCHAR_TO_ANSI(*EventName), EventID, (EventCallBack), (t_CKBOOL)(true));
	
	
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
