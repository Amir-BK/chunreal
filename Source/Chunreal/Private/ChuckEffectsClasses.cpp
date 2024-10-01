#include "ChuckEffectsClasses.h"

inline void FSourceEffectChuck::OnPresetChanged()
{
	GET_EFFECT_SETTINGS(SourceEffectChuck)


	if (ChuckProcessor != Settings.ChuckInstance)
	{
		DeleteOurChuck();
		ChuckProcessor = Settings.ChuckInstance;
		if (IsValid(ChuckProcessor))
		{
			ChuckRef = ChuckProcessor->SpawnChuckFromAsset(FString(), SampleRate, NumChannels);
			ChuckRef->init();
			ChuckRef->start();
		}
	}

	//so we should only enter this block if we have a valid chuck processor
	if (ChuckRef)
	{
		//if our GUID has changed, we need to recompile the chuck
		if (CurrentChuckGuid != Settings.ChuckInstance->ChuckGuid)
		{
			if (bHasSporkedOnce)
			{
				Chuck_Msg* msg = new Chuck_Msg;
				msg->type = 3;  //MSG_REMOVEALL
				ChuckRef->vm()->process_msg(msg);
			}
			else
			{
				bHasSporkedOnce = true;
			}
			
			CurrentChuckGuid = Settings.ChuckInstance->ChuckGuid;
			ChuckProcessor->CompileChuckAsset(ChuckRef);

			//we should be good to go now, we'll deal with the parameters later, in theory we shouldn't recompile the chuck if the parameters change, only if the Guid does.
		}

		for (const auto& [Key, Param] : Settings.Params)
		{

			const auto& TypeName = Param.ParamType;
			switch (Param.ParamType)
			{
			case EAudioParameterType::Integer:
				ChuckRef->globals()->setGlobalInt(TCHAR_TO_ANSI(*Key.ToString()), Param.IntParam);
				break;
			case EAudioParameterType::Float:
				ChuckRef->globals()->setGlobalFloat(TCHAR_TO_ANSI(*Key.ToString()), Param.FloatParam);
				break;
			case EAudioParameterType::String:
				ChuckRef->globals()->setGlobalString(TCHAR_TO_ANSI(*Key.ToString()), TCHAR_TO_UTF8(*Param.StringParam));
				break;
			case EAudioParameterType::Boolean:
				ChuckRef->globals()->setGlobalInt(TCHAR_TO_ANSI(*Key.ToString()), Param.BoolParam);
				break;
			default:
				break;
			}


		}
	}



}

void FSubmixChuckEffect::OnPresetChanged()
{
	GET_EFFECT_SETTINGS(SubmixChuckEffect)


		if (ChuckProcessor != Settings.ChuckInstance)
		{
			DeleteOurChuck();
			ChuckProcessor = Settings.ChuckInstance;
			if (IsValid(ChuckProcessor))
			{
				ChuckRef = ChuckProcessor->SpawnChuckFromAsset(FString(), SampleRate, NumChannels);
				ChuckRef->init();
				ChuckRef->start();
			}
		}

	//so we should only enter this block if we have a valid chuck processor
	if (ChuckRef)
	{
		//if our GUID has changed, we need to recompile the chuck
		if (CurrentChuckGuid != Settings.ChuckInstance->ChuckGuid)
		{
			if (bHasSporkedOnce)
			{
				Chuck_Msg* msg = new Chuck_Msg;
				msg->type = 3;  //MSG_REMOVEALL
				ChuckRef->vm()->process_msg(msg);
			}
			else
			{
				bHasSporkedOnce = true;
			}

			CurrentChuckGuid = Settings.ChuckInstance->ChuckGuid;
			ChuckProcessor->CompileChuckAsset(ChuckRef);


			//we should be good to go now, we'll deal with the parameters later, in theory we shouldn't recompile the chuck if the parameters change, only if the Guid does.
		}

		for (const auto& [Key, Param] : Settings.Params)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Key: %s"), *Key.ToString());
			
			const auto& TypeName = Param.ParamType;
			switch (Param.ParamType)
			{
			case EAudioParameterType::Integer:
				ChuckRef->globals()->setGlobalInt(TCHAR_TO_ANSI(*Key.ToString()), Param.IntParam);
				break;
			case EAudioParameterType::Float:
				ChuckRef->globals()->setGlobalFloat(TCHAR_TO_ANSI(*Key.ToString()), Param.FloatParam);
				break;
			case EAudioParameterType::String:
				ChuckRef->globals()->setGlobalString(TCHAR_TO_ANSI(*Key.ToString()), TCHAR_TO_UTF8(*Param.StringParam));
				break;
			case EAudioParameterType::Boolean:
				ChuckRef->globals()->setGlobalInt(TCHAR_TO_ANSI(*Key.ToString()), Param.BoolParam);
				break;
			default:
				break;
			}


		}
	}

}
