// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GunnerAnimInstance.generated.h"

class UGunnerLocomotionAnimSet;
/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	void SetLocomotionAnimSet(UGunnerLocomotionAnimSet* InLocomotionAnimSet);
	void ClearLocomotionAnimSet();
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGunnerLocomotionAnimSet> DefaultLocomotionAnimSet;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess	= "true"))
	TObjectPtr<UGunnerLocomotionAnimSet> LocomotionAnimSet;
};
