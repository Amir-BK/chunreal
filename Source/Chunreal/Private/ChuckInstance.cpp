// Fill out your copyright notice in the Description page of Project Settings.


#include "ChuckInstance.h"

// Inherited via IAudioProxyDataFactory

DEFINE_METASOUND_DATA_TYPE(Metasound::FChuckInstance, "ChucK Instance")

inline TSharedPtr<Audio::IProxyData> UChuckInstance::CreateProxyData(const Audio::FProxyDataInitParams& InitParams)
{
	if (!bChuckCompiled)
	{
		Chuck = new ChucK();

		//Initialize Chuck params
		Chuck->setParam(CHUCK_PARAM_SAMPLE_RATE, FChunrealModule::GetChuckSampleRate());
		Chuck->setParam(CHUCK_PARAM_INPUT_CHANNELS, 2);
		Chuck->setParam(CHUCK_PARAM_OUTPUT_CHANNELS, 2);
		Chuck->setParam(CHUCK_PARAM_VM_ADAPTIVE, 0);
		Chuck->setParam(CHUCK_PARAM_VM_HALT, (t_CKINT)(false));
		//Chuck->setParam(CHUCK_PARAM_OTF_PORT, g_otf_port);
		//Chuck->setParam(CHUCK_PARAM_OTF_ENABLE, (t_CKINT)TRUE);
		//Chuck->setParam(CHUCK_PARAM_DUMP_INSTRUCTIONS, (t_CKINT)dump);
		Chuck->setParam(CHUCK_PARAM_AUTO_DEPEND, (t_CKINT)0);
		//Chuck->setParam(CHUCK_PARAM_DEPRECATE_LEVEL, deprecate_level);
		Chuck->setParam(CHUCK_PARAM_CHUGIN_ENABLE, false);
		//Chuck->setParam(CHUCK_PARAM_USER_CHUGINS, named_dls);
		//Chuck->setParam(CHUCK_PARAM_USER_CHUGIN_DIRECTORIES, dl_search_path);
		Chuck->setParam(CHUCK_PARAM_HINT_IS_REALTIME_AUDIO, true);

		Chuck->init();
		Chuck->start();

		//Store ChucK reference with ID
		if (!((FString)(*Id)).IsEmpty())
		{
			FChunrealModule::StoreChuckRef(Chuck, *Id);
			//FChunrealModule::Log(FString("Added ChucK ID: ") + **ID);
		}

		if (bHasSporkedOnce)
		{
			Chuck_Msg* msg = new Chuck_Msg;
			msg->type = 3;  //MSG_REMOVEALL
			Chuck->vm()->process_msg(msg);
		}
		else
		{
			bHasSporkedOnce = true;
		}
		FChunrealModule::CompileChuckCode(Chuck, TCHAR_TO_UTF8(*Code));
	}

	bool bHasSuccesfullyCopiled = (bool) Chuck->vm_running();
	FString CompileText = bHasSuccesfullyCopiled ? "Compiled" : "Failed to compile";

	UE_LOG(LogTemp, Log, TEXT("ChucK compiled: %s"), *CompileText);

	return MakeShared<FChuckInstanceProxy>(Chuck, Id);
}
