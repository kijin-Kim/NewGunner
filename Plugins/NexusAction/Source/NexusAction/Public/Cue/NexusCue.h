// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NexusCue.generated.h"

USTRUCT(BlueprintType)
struct FNexusTargetDataBase
{
	GENERATED_BODY()

public:
	virtual UScriptStruct* GetStructType() const { return FNexusTargetDataBase::StaticStruct(); }
	virtual ~FNexusTargetDataBase() = default;
};


USTRUCT(BlueprintType)
struct FNexusRepDataHandle
{
	GENERATED_BODY()

public:
	FNexusRepDataHandle() = default;
	FNexusRepDataHandle(const FNexusRepDataHandle& Other) : Data(Other.Data) {}
	FNexusRepDataHandle(FNexusRepDataHandle&& Other) : Data(MoveTemp(Other.Data)) {}
	
	FNexusRepDataHandle& operator=(FNexusRepDataHandle&& Other)
	{
		Data = MoveTemp(Other.Data);
		return *this;
	}

	FNexusRepDataHandle& operator=(const FNexusRepDataHandle& Other)
	{
		Data = Other.Data;
		return *this;
	}

	void SetData(TSharedPtr<FNexusTargetDataBase> InData) { Data = InData; }
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
	TSharedPtr<FNexusTargetDataBase> GetData() const { return Data; }

private:
	TSharedPtr<FNexusTargetDataBase> Data; // 모든 카피가 Destruct될 때까지 유지되어야함.
};

template <>
struct TStructOpsTypeTraits<FNexusRepDataHandle> : public TStructOpsTypeTraitsBase2<FNexusRepDataHandle>
{
	enum
	{
		WithCopy = true, // 디폴트로 Memcopy를 사용하여 복사되기 때문에 TSharedPtr의 참조카운터를 증가시키기위해 필요함
		WithNetSerializer = true
	};
};


/**
 * 
 */
UCLASS(Blueprintable, meta = (ShowWorldContextPin))
class NEXUSACTION_API UNexusCue : public UObject
{
	GENERATED_BODY()

public:
	virtual UWorld* GetWorld() const override;
	UFUNCTION(BlueprintNativeEvent)
	void OnTriggered();
	void SetCueRepData(const FNexusRepDataHandle& InRepDataHandle) { RepDataHandle = InRepDataHandle; }
	
private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	FNexusRepDataHandle RepDataHandle;
};
