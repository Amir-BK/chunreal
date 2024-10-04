// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "IAudioProxyInitializer.h"
#include "MetasoundDataTypeRegistrationMacro.h"
#include "Chunreal.h"
//#include "Chunreal/chuck/chuck.h"
#include "ChuckInstance.generated.h"


//class FChuckInstanceProxy;
/**
 * A chuck processor really represents the CODE for a chuck, not the actual running instance of a chuck,
 * So it's more of a template and I might rename it to reflect this
 */
UCLASS(BlueprintType)
class CHUNREAL_API UChuckProcessor : public UObject, public IAudioProxyDataFactory
{
	GENERATED_BODY()

	// Inherited via IAudioProxyDataFactory
	virtual TSharedPtr<Audio::IProxyData> CreateProxyData(const Audio::FProxyDataInitParams& InitParams) override;

public:

	UPROPERTY()
	bool bIsAutoManaged = false; //for chucks that are automatically created by the system from .ck files in the working directory

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chuck")
	FString SourcePath;

	UPROPERTY()
	FGuid ChuckGuid;


	bool CompileChuckCode();

	bool bNeedsRecompile = false;

	UPROPERTY(BlueprintReadWrite, Category = "Chuck", meta = (MultiLine = true, ExposeOnSpawn = true))
	FString Code;

	//if true, the ChucK will only be compiled once and shared with all instantiations via metasounds 
	UPROPERTY(BlueprintReadWrite, Category = "Chuck", meta = (ExposeOnSpawn = true), EditAnywhere)
	bool bShareChuck = false;

	//spawn chuck with optional instance ID for registration with the module, we'll see about destroying it later
	ChucK* SpawnChuckFromAsset(FString InstanceID = FString(), int32 InSampleRate = 48000, int32 InNumChannels = 2);

	void CompileChuckAsset(ChucK* chuckRef);

	//TMap<FString, Chuck*> ChuckInstances;

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

	explicit FChuckCodeProxy(UChuckProcessor* InChuckProcessor)
		: 
		ChuckProcessor(InChuckProcessor)
	{
	}

	FChuckCodeProxy(const FChuckCodeProxy& Other) = default;

	UChuckProcessor* ChuckProcessor = nullptr;
	//FString ChuckCode;


};

//this should represent a live instance of a chuck vm, it is not meant to be shared by sound generators as this will corrupt the buffers
//Differentiating between the two kind of objects lets us use Chucks as templates while also having access to their parameters, from metasound as well as BP and code.
UCLASS(BlueprintType)
class CHUNREAL_API UChuckInstantiation : public UChuckProcessor
{
	GENERATED_BODY()

	ChucK* ChuckInstance = nullptr;

};

namespace Metasound
{
	class CHUNREAL_API FChuckInstance
	{
	public:
		FChuckInstance() = default;
		FChuckInstance(const FChuckInstance&) = default;
		FChuckInstance& operator=(const FChuckInstance& Other) = default;
		FChuckInstance(const TSharedPtr<Audio::IProxyData>& InInitData)
		{
			ChuckProxy = StaticCastSharedPtr<FChuckCodeProxy>(InInitData);
		}

		bool IsInitialized() const { return ChuckProxy.IsValid(); }

		//void RegenerateInputs() const { ChuckProxy->RegenerateInputs(); }

		const FChuckCodeProxy* GetProxy() const { return ChuckProxy.Get(); }

	private:
		TSharedPtr<FChuckCodeProxy, ESPMode::ThreadSafe> ChuckProxy;
	};

	DECLARE_METASOUND_DATA_REFERENCE_TYPES(FChuckInstance, CHUNREAL_API, FChuckInstanceTypeInfo, FChuckInstanceReadRef, FChuckInstanceWriteRef)


}