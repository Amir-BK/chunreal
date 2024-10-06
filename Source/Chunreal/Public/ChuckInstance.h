// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "IAudioProxyInitializer.h"
#include "MetasoundDataTypeRegistrationMacro.h"
#include "Chunreal.h"
//#include "Chunreal/chuck/chuck.h"
#include "ChuckInstance.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnChuckNeedsRecompile);

class UChuckInstantiation;

//class FChuckInstanceProxy;
/**
 * A chuck processor really represents the CODE for a chuck, not the actual running instance of a chuck,
 * So it's more of a template and I might rename it to reflect this
 */
UCLASS(BlueprintType)
class CHUNREAL_API UChuckCode : public UObject, public IAudioProxyDataFactory
{
	GENERATED_BODY()

	// Inherited via IAudioProxyDataFactory
	virtual TSharedPtr<Audio::IProxyData> CreateProxyData(const Audio::FProxyDataInitParams& InitParams) override;

public:

	UPROPERTY()
	bool bIsAutoManaged = false; //for chucks that are automatically created by the system from .ck files in the working directory

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chuck")
	FString SourcePath;




	bool CompileChuckCode();

	bool bNeedsRecompile = false;

	UPROPERTY(BlueprintReadWrite, Category = "Chuck", meta = (MultiLine = true, ExposeOnSpawn = true))
	FString Code;

	//if true, the ChucK will only be compiled once and shared with all instantiations via metasounds 
	UPROPERTY(BlueprintReadWrite, Category = "Chuck", meta = (ExposeOnSpawn = true), EditAnywhere)
	bool bShareChuck = false;

	//spawn chuck with optional instance ID for registration with the module, we'll see about destroying it later
	ChucK* CreateChuckVm(int32 InNumChannels = 2);

	UFUNCTION(BlueprintCallable, Category = "ChucK")
	UChuckInstantiation* SpawnChuckInstance(int32 InSampleRate = 48000, int32 InNumChannels = 2);

	void CompileChuckAsset(ChucK* chuckRef);

	//TMap<FString, Chuck*> ChuckInstances;

	FOnChuckNeedsRecompile OnChuckNeedsRecompile;


private:
	ChucK* Chuck = nullptr;

	bool bChuckCompiled = false;
	bool bHasSporkedOnce = false;
	std::shared_ptr<ChucK*> ChuckPtr = nullptr;

	
};

class CHUNREAL_API FChuckCodeProxy : public Audio::TProxyData<FChuckCodeProxy>
{
public:
	IMPL_AUDIOPROXY_CLASS(FChuckCodeProxy);

	explicit FChuckCodeProxy(UChuckCode* InChuckProcessor)
		: 
		ChuckProcessor(InChuckProcessor)
	{
	}

	FChuckCodeProxy(const FChuckCodeProxy& Other) = default;

	UChuckCode* ChuckProcessor = nullptr;
	//FString ChuckCode;


};

//this should represent a live instance of a chuck vm, it is not meant to be shared by sound generators as this will corrupt the buffers
//Differentiating between the two kind of objects lets us use Chucks as templates while also having access to their parameters, from metasound as well as BP and code.
UCLASS(BlueprintType, Transient, Within = ChuckCode)
class CHUNREAL_API UChuckInstantiation : public UObject, public IAudioProxyDataFactory
{
	GENERATED_BODY()

	UChuckInstantiation()
	{
		// Must be created by a valid ChuckCode object
		if (!IsTemplate()) {
			ParentChuckCode = CastChecked<UChuckCode>(GetOuter());
			ParentChuckCode->OnChuckNeedsRecompile.AddUObject(this, &UChuckInstantiation::OnChuckCodeAssetChanged);
			CompileCode();
		}
	}

	~UChuckInstantiation()
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
	TArray<FAudioParameter> InputParameters;
	TArray<FAudioParameter> OutputParameters;
public:
	// Inherited via IAudioProxyDataFactory
	virtual TSharedPtr<Audio::IProxyData> CreateProxyData(const Audio::FProxyDataInitParams& InitParams) override;

	void CreateChuckVm(int32 InNumChannels = 2)
	{
		ChuckVm = ParentChuckCode->CreateChuckVm(InNumChannels);
		ChuckVm->init();
		ChuckVm->start();

		//CompileCode();
	}

	void CompileCode()
	{
		UE_LOG(LogTemp,Error, TEXT("Is this shit being called twice in a row???"))
		
		//is garbage?
		if (!IsValid(this)) return; 
		
		if (ChuckVm == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("ChuckVm is null"));
			CreateChuckVm();
		}


		if (bHasSporkedOnce)
		{
			Chuck_Msg* msg = new Chuck_Msg;
			msg->type = 3;  //MSG_REMOVEALL
			ChuckVm->vm()->process_msg(msg);
		}
		else
		{
			bHasSporkedOnce = true;
		}
		
		if (ParentChuckCode->bIsAutoManaged)
		{
			//FChunrealModule ChunrealModule = FModuleManager::Get().GetModuleChecked<FChunrealModule>("Chunreal");
			//FString WorkingDir = ChunrealModule.workingDirectory;
			FChunrealModule::CompileChuckFile(ChuckVm, TCHAR_TO_UTF8(*ParentChuckCode->SourcePath));

		}
		else
		{
			FChunrealModule::CompileChuckCode(ChuckVm, TCHAR_TO_UTF8(*ParentChuckCode->Code));
		}



	}

	void OnChuckCodeAssetChanged()
	{
		UE_LOG(LogTemp, Warning, TEXT("Chuck Code Asset Changed"));
		if (ChuckVm != nullptr)
		{
			CompileCode();
		}
	}

	//doesn't actually work, in theory it could be used to map all the i/os from a chuck, maybe should be attempted using ckdoc class
	UFUNCTION(BlueprintCallable, Category = "ChucK")
	TArray<FAudioParameter> GetAllGlobalOutputsFromChuck();

	// the chuck code object used to spawn this instance, 
	UPROPERTY()
	TObjectPtr<UChuckCode> ParentChuckCode;

	bool bHasSporkedOnce = false;

	



public:
	//probably shoulnd't be public
	ChucK* ChuckVm = nullptr;

};

//proxy for Instantiations
class CHUNREAL_API FChuckInstanceProxy : public Audio::TProxyData<FChuckInstanceProxy>
{
public:
	IMPL_AUDIOPROXY_CLASS(FChuckInstanceProxy);

	explicit FChuckInstanceProxy(UChuckInstantiation* InChuckInstance)
		:
		ChuckInstance(InChuckInstance)
	{
	}

	FChuckInstanceProxy(const FChuckInstanceProxy& Other) = default;

	UChuckInstantiation* ChuckInstance = nullptr;
	//FString ChuckCode;


};

namespace Metasound
{
	class CHUNREAL_API FChuckProcessor
	{
	public:
		FChuckProcessor() = default;
		FChuckProcessor(const FChuckProcessor&) = default;
		FChuckProcessor& operator=(const FChuckProcessor& Other) = default;
		FChuckProcessor(const TSharedPtr<Audio::IProxyData>& InInitData)
		{
			ChuckProxy = StaticCastSharedPtr<FChuckCodeProxy>(InInitData);
		}

		bool IsInitialized() const { return ChuckProxy.IsValid(); }

		//void RegenerateInputs() const { ChuckProxy->RegenerateInputs(); }

		const FChuckCodeProxy* GetProxy() const { return ChuckProxy.Get(); }

	private:
		TSharedPtr<FChuckCodeProxy, ESPMode::ThreadSafe> ChuckProxy;
	};

	DECLARE_METASOUND_DATA_REFERENCE_TYPES(FChuckProcessor, CHUNREAL_API, FChuckProcessorTypeInfo, FChuckProcessorReadRef, FChuckProcessorWriteRef)

	class CHUNREAL_API FChuckInstance
	{
	public:
		FChuckInstance() = default;
		FChuckInstance(const FChuckInstance&) = default;
		FChuckInstance& operator=(const FChuckInstance& Other) = default;
		FChuckInstance(const TSharedPtr<Audio::IProxyData>& InInitData)
		{
			ChuckProxy = StaticCastSharedPtr<FChuckInstanceProxy>(InInitData);
		}

		bool IsInitialized() const { return ChuckProxy.IsValid(); }

		//void RegenerateInputs() const { ChuckProxy->RegenerateInputs(); }

		const FChuckInstanceProxy* GetProxy() const { return ChuckProxy.Get(); }

	private:
		TSharedPtr<FChuckInstanceProxy, ESPMode::ThreadSafe> ChuckProxy;
	};

	DECLARE_METASOUND_DATA_REFERENCE_TYPES(FChuckInstance, CHUNREAL_API, FChuckInstanceTypeInfo, FChuckInstanceReadRef, FChuckInstanceWriteRef)
	
}