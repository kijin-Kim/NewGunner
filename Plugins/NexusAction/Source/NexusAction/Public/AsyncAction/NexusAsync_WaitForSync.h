// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NexusAsync.h"
#include "Delegates/Delegate.h"
#include "NexusAsync_WaitForSync.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNexusSyncSignature);

/**
 *  클라이언트와 서버 사이의 동기화 포인트를 생성합니다.
 *  클라이언트: 예측 구간을 생성하고 바로 진행합니다.
 *  서버: 클라이언트로부터 예측 태그를 받은 후 예측 구간을 생성하고 진행합니다. 
 */
UCLASS()
class NEXUSACTION_API UNexusAsync_WaitForSync : public UNexusAsync
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (HidePin = "InAction", DefaultToSelf = "InAction", BlueprintInternalUseOnly = "true"))
	static UNexusAsync_WaitForSync* WaitForSync(UNexusAction* InAction);

	//~ Begin UCancellableAsyncAction Interface.
	virtual void Activate() override;
	//~ End UCancellableAsyncAction Interface.

	void OnSync();

public:
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnSync"))
	FOnNexusSyncSignature OnSyncDelegate;
};
