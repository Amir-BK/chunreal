// Fill out your copyright notice in the Description page of Project Settings.


#include "ChuckInstance.h"
#include "Chunreal/chuck/chuck_globals.h"
#include "HAL/PlatformApplicationMisc.h"

// Inherited via IAudioProxyDataFactory

DEFINE_LOG_CATEGORY_STATIC(LogChuckInstance, VeryVerbose, All);

DEFINE_METASOUND_DATA_TYPE(Metasound::FChuckProcessor, "ChucK Processor")
DEFINE_METASOUND_DATA_TYPE(Metasound::FChuckInstance, "ChucK Instance")

inline TSharedPtr<Audio::IProxyData> UChuckProcessor::CreateProxyData(const Audio::FProxyDataInitParams& InitParams)
{

	return MakeShared<FChuckCodeProxy>(this);
}

bool UChuckProcessor::CompileChuckCode()
{
	const auto PlatformAudioSettings = FAudioPlatformSettings::GetPlatformSettings(FPlatformProperties::GetRuntimeSettingsClassName());
	const auto PlatformSampleRate = PlatformAudioSettings.SampleRate;
	const auto ExpectedBlockSize = PlatformAudioSettings.CallbackBufferFrameSize;
	const auto ExpectedNumBlocks = PlatformAudioSettings.NumBuffers;
	
	
	
	return false;
}

ChucK* UChuckProcessor::SpawnChuckFromAsset(FString InstanceID, int32 InSampleRate, int32 InNumChannels)
{
	ChucK* theChuck = new ChucK();
	
	//UE_LOG(LogChucKMidiNode, VeryVerbose, TEXT("Creating new chuck for asset: %s"));

	theChuck = new ChucK();
	theChuck->setLogLevel(8);
	//Initialize Chuck params
	theChuck->setParam(CHUCK_PARAM_SAMPLE_RATE, InSampleRate);
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

UChuckInstantiation* UChuckProcessor::SpawnChuckInstance(int32 InSampleRate, int32 InNumChannels)
{
	auto* NewChuck = NewObject<UChuckInstantiation>(this);
	NewChuck->ChuckInstance = SpawnChuckFromAsset(FString(), InSampleRate, InNumChannels);
	//need to init
	NewChuck->ChuckInstance->init();
	NewChuck->ChuckInstance->start();
	CompileChuckAsset(NewChuck->ChuckInstance);
	
	return NewChuck;
}

void UChuckProcessor::CompileChuckAsset(ChucK* chuckRef)
{
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

inline TArray<FAudioParameter> UChuckInstantiation::GetAllGlobalOutputsFromChuck() {
	check(ChuckInstance != nullptr);
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
	
	ChuckInstance->globals()->getAllGlobalVariables(MyCallbackFunction, data);

	delete static_cast<int*>(data);

	return Params;
}
