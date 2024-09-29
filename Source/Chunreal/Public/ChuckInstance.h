// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "IAudioProxyInitializer.h"
#include "MetasoundDataTypeRegistrationMacro.h"
#include "Chunreal.h"
#include "ChuckInstance.generated.h"


//class FChuckInstanceProxy;
/**
 * 
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


	bool CompileChuckCode();

	UPROPERTY(BlueprintReadWrite, Category = "Chuck", meta = (MultiLine = true, ExposeOnSpawn = true))
	FString Code;

	//if true, the ChucK will only be compiled once and shared with all instantiations via metasounds 
	UPROPERTY(BlueprintReadWrite, Category = "Chuck", meta = (ExposeOnSpawn = true), EditAnywhere)
	bool bShareChuck = false;

private:
	ChucK* Chuck = nullptr;

	bool bChuckCompiled = false;
	bool bHasSporkedOnce = false;
	std::shared_ptr<ChucK*> ChuckPtr = nullptr;

	
};

class CHUNREAL_API FChuckInstanceProxy : public Audio::TProxyData<FChuckInstanceProxy>
{
public:
	IMPL_AUDIOPROXY_CLASS(FChuckInstanceProxy);

	explicit FChuckInstanceProxy(UChuckProcessor* InChuckProcessor)
		: 
		ChuckProcessor(InChuckProcessor)
	{
	}

	FChuckInstanceProxy(const FChuckInstanceProxy& Other) = default;

	UChuckProcessor* ChuckProcessor = nullptr;
	//FString ChuckCode;


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