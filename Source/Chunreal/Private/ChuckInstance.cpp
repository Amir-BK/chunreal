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
	static int EventIdCounter = 0;
}

inline TSharedPtr<Audio::IProxyData> UChuckCode::CreateProxyData(const Audio::FProxyDataInitParams& InitParams)
{

	return MakeShared<FChuckCodeProxy>(this);
}

bool UChuckCode::CompileChuckCode()
{
	const auto PlatformAudioSettings = FAudioPlatformSettings::GetPlatformSettings(FPlatformProperties::GetRuntimeSettingsClassName());
	const auto PlatformSampleRate = PlatformAudioSettings.SampleRate;
	const auto ExpectedBlockSize = PlatformAudioSettings.CallbackBufferFrameSize;
	const auto ExpectedNumBlocks = PlatformAudioSettings.NumBuffers;
	
	
	
	return false;
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



void UChuckInstantiation::SubscribeToGlobalEvent(FString EventName, const FOnGlobalEventExecuted& InDelegate) {
	// I think we don't need to check if is valid... we'll see
	t_CKINT EventID = ChunrealEventRegistry::EventIdCounter++;
	auto EventCallBack = [](t_CKINT inEventID) {
		//InDelegate.Execute(EventName);
		UE_LOG(LogChuckInstance, VeryVerbose, TEXT("Event executed %d"), inEventID);
		auto EventTuple = ChunrealEventRegistry::EventDelegates[inEventID];
		EventTuple.Value.ExecuteIfBound(EventTuple.Key);
		};

	const char* EventNameChar = TCHAR_TO_ANSI(*EventName);
	//add to global static map
	auto EventTuple = TTuple<FString, FOnGlobalEventExecuted>(EventName, InDelegate);
	ChunrealEventRegistry::EventDelegates.Add(TTuple<t_CKINT, TTuple<FString, FOnGlobalEventExecuted>>(EventID, EventTuple));

	ChuckVm->globals()->listenForGlobalEvent(EventNameChar, EventID, (EventCallBack), (t_CKBOOL)(true));

};

UChuckInstantiation* UChuckCode::SpawnChuckInstance(int32 InSampleRate, int32 InNumChannels)
{
	const auto PlatformAudioSettings = FAudioPlatformSettings::GetPlatformSettings(FPlatformProperties::GetRuntimeSettingsClassName());
	const auto PlatformSampleRate = PlatformAudioSettings.SampleRate;
	
	auto* NewChuck = NewObject<UChuckInstantiation>(this);
	//NewChuck->CreateChuckVm(InNumChannels);
	//NewChuck->ChuckVm = CreateChuckVm(InNumChannels);
	//need to init
	//NewChuck->ChuckVm->init();
	//NewChuck->ChuckVm->start();
	//CompileChuckAsset(NewChuck->ChuckVm);
	
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

inline int32 UChuckInstantiation::OnGenerateAudio(float* OutAudio, int32 NumSamples) {
	//so if we're here we must already have a chuck vm, I think? 
	FChunrealModule::RunChuck(ChuckVm, nullptr, OutAudio, NumSamples);

	return NumSamples;
}

TSharedPtr<Audio::IProxyData> UChuckInstantiation::CreateProxyData(const Audio::FProxyDataInitParams& InitParams)
{
	return MakeShared<FChuckInstanceProxy>(this);
}

inline TArray<FAudioParameter> UChuckInstantiation::GetAllGlobalOutputsFromChuck() {
	check(ChuckVm != nullptr);
	UE_LOG(LogChuckInstance, VeryVerbose, TEXT("Getting all global outputs from Chuck instance"));
	TArray<FAudioParameter> Params;

	// Define the callback function
	auto MyCallbackFunction = [](const std::vector<Chuck_Globals_TypeValue>& list, void* data) {
		// Process the list of global variables
		UE_LOG(LogChuckInstance, VeryVerbose, TEXT("Processing list of global variables"));
		for (const auto& item : list) {
			// Example: Print the name of each global variable
			UE_LOG(LogChuckInstance, VeryVerbose, TEXT("Global Variable: %s"), *FString(item.name.c_str()));
		}
		};

	// Call the getAllGlobalVariables method
	void* data = new long;
	
	ChuckVm->globals()->getAllGlobalVariables(MyCallbackFunction, data);

	delete static_cast<int*>(data);

	return Params;
}
