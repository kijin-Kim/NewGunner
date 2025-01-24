// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GunnerActionSign.generated.h"

USTRUCT(BlueprintType)
struct FGunnerTargetDataBase
{
	GENERATED_BODY()

public:
	virtual UScriptStruct* GetStructType() const { return FGunnerTargetDataBase::StaticStruct(); }
	virtual ~FGunnerTargetDataBase() = default;
};


USTRUCT(BlueprintType)
struct FGunnerTargetDataHandle
{
	GENERATED_BODY()

public:
	FGunnerTargetDataHandle() = default;
	FGunnerTargetDataHandle(const FGunnerTargetDataHandle& Other) : Data(Other.Data) {}
	FGunnerTargetDataHandle(FGunnerTargetDataHandle&& Other) : Data(MoveTemp(Other.Data)) {}
	
	FGunnerTargetDataHandle& operator=(FGunnerTargetDataHandle&& Other)
	{
		Data = MoveTemp(Other.Data);
		return *this;
	}

	FGunnerTargetDataHandle& operator=(const FGunnerTargetDataHandle& Other)
	{
		Data = Other.Data;
		return *this;
	}

	void SetData(TSharedPtr<FGunnerTargetDataBase> InData) { Data = InData; }
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
	TSharedPtr<FGunnerTargetDataBase> GetData() const { return Data; }

private:
	TSharedPtr<FGunnerTargetDataBase> Data; // FGunnerSignalTargetDataHandle의 모든 카피가 Destruct될 때까지 유지되어야함.
};

template <>
struct TStructOpsTypeTraits<FGunnerTargetDataHandle> : public TStructOpsTypeTraitsBase2<FGunnerTargetDataHandle>
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
class GUNNER_API UGunnerActionSign : public UObject
{
	GENERATED_BODY()

public:
	virtual UWorld* GetWorld() const override;
	UFUNCTION(BlueprintNativeEvent)
	void OnSignaled();
	void SetSignalDataObject(UObject* InSignalDataObject) { SignalDataObject = InSignalDataObject; }
	void SetSignalTargetData(const FGunnerTargetDataHandle& InSignalTargetData) { SignalTargetData = InSignalTargetData; }
	UFUNCTION(BlueprintCallable)
	UObject* GetSignalDataObject() const { return SignalDataObject.Get(); }

private:
	TWeakObjectPtr<UObject> SignalDataObject;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	FGunnerTargetDataHandle SignalTargetData;
};
