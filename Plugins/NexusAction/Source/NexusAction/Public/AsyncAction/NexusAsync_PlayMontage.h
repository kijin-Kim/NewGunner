// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NexusAsync.h"
#include "NexusAsync_PlayMontage.generated.h"

class UNexusAction;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMontageEventSiganture);

/**
 * 
 */
UCLASS()
class NEXUSACTION_API UNexusAsync_PlayMontage : public UNexusAsync
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, meta = (HidePin = "InAction", DefaultToSelf = "InAction", BlueprintInternalUseOnly = "true"))
	static UNexusAsync_PlayMontage* PlayMontage(UNexusAction* InAction, AActor* MontageActor, UAnimMontage* MontageToPlay, bool bIsThirdPerson = false, float PlayRate = 1.0f, FName StartSectionName = NAME_None, bool bStopWhenActionEnds = false);
	//~ Begin UCancellableAsyncAction Interface.
	virtual bool ShouldBroadcastDelegates() const override;
	virtual void Activate() override;
	virtual void SetReadyToDestroy() override;
	//~ End UCancellableAsyncAction Interface.
	
public:
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnBlendOut"))
	FOnMontageEventSiganture OnBlendOutDelegate;
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnInterrupted"))
	FOnMontageEventSiganture OnInterruptedDelegate;
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnCompleted"))
	FOnMontageEventSiganture OnCompletedDelegate;

private:
	TWeakObjectPtr<AActor> MontageActor;
	TWeakObjectPtr<UAnimMontage> MontageToPlay;
	bool bIsThirdPerson;
	float PlayRate;
	FName StartSectionName;
	bool bStopWhenActionEnds;
};
