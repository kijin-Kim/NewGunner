// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerCancellableAsyncAction.h"
#include "GunnerActionAsync_PlayMontage.generated.h"

class UGunnerAction;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMontageEventSiganture);

/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerActionAsync_PlayMontage : public UGunnerCancellableAsyncAction
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, meta = (HidePin = "InAction", DefaultToSelf = "InAction", BlueprintInternalUseOnly = "true"))
	static UGunnerActionAsync_PlayMontage* PlayMontage(UGunnerAction* InAction, AActor* MontageActor, UAnimMontage* MontageToPlay, bool bIsThirdPerson = false, float PlayRate = 1.0f, FName StartSectionName = NAME_None);
	//~ Begin UCancellableAsyncAction Interface.
	virtual void Activate() override;
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
};
