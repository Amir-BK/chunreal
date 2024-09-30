// Fill out your copyright notice in the Description page of Project Settings.


#include "ChuckInstance.h"
#include "HAL/PlatformApplicationMisc.h"

// Inherited via IAudioProxyDataFactory

DEFINE_LOG_CATEGORY_STATIC(LogChuckInstance, VeryVerbose, All);

DEFINE_METASOUND_DATA_TYPE(Metasound::FChuckInstance, "ChucK Instance")

inline TSharedPtr<Audio::IProxyData> UChuckProcessor::CreateProxyData(const Audio::FProxyDataInitParams& InitParams)
{

	return MakeShared<FChuckInstanceProxy>(this);
}

bool UChuckProcessor::CompileChuckCode()
{
	const auto PlatformAudioSettings = FAudioPlatformSettings::GetPlatformSettings(FPlatformProperties::GetRuntimeSettingsClassName());
	const auto PlatformSampleRate = PlatformAudioSettings.SampleRate;
	const auto ExpectedBlockSize = PlatformAudioSettings.CallbackBufferFrameSize;
	const auto ExpectedNumBlocks = PlatformAudioSettings.NumBuffers;
	
	
	
	return false;
}
