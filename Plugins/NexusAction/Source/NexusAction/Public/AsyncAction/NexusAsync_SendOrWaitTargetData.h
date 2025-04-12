// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NexusAsync.h"
#include "Delegates/Delegate.h"
#include "NexusAsync_SendOrWaitTargetData.generated.h"




DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNexusTargetDataArrivedSignature, FNexusTargetDataHandle, TargetDataHandle);
/**
 * 클라이언트에서 타깃 데이터를 보내고, 서버에서 타깃 데이터를 기다립니다. 클라이언트와 서버 사이의 동기화 포인트가 생성됩니다.
 * 클라이언트: 타깃 데이터와 예측 태그를 보내고, 예측 구간을 생성한 후 바로 진행합니다.
 * 서버: 클라이언트로부터 타깃 데이터와 예측 태그를 기다린 후, 예측 구간을 생성하고 진행합니다. 
 */
UCLASS()
class NEXUSACTION_API UNexusAsync_SendOrWaitTargetData : public UNexusAsync
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, meta = (HidePin = "InAction", DefaultToSelf = "InAction", BlueprintInternalUseOnly = "true"))
	static UNexusAsync_SendOrWaitTargetData* SendOrWaitTargetData(UNexusAction* InAction, const FNexusTargetDataHandle& InTargetDataHandle);
	
	//~ Begin UCancellableAsyncAction Interface.
	virtual void Activate() override;
	//~ End UCancellableAsyncAction Interface.
	
	void OnArrived(FNexusTargetDataHandle InTargetDataHandle);

public:
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnArrived"))
	FOnNexusTargetDataArrivedSignature OnArrivedDelegate;

private:
	FNexusTargetDataHandle TargetDataHandle;
};
