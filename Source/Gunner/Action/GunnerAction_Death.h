// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Action/NexusAction.h"
#include "Gunner/_Core/GunnerDirectionalMontage.h"
#include "Gunner/_Core/GunnerHitBoxInterface.h"
#include "GunnerAction_Death.generated.h"


struct FGunnerDamageContext;


/**
 * 
 */
UCLASS(Abstract)
class GUNNER_API UGunnerAction_Death : public UNexusAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	UAnimMontage* GetDesiredDeathMontage(bool bLarge) const;

protected:
	virtual void OnTriggerAction() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TMap<EGunnerHitPartType, FGunnerDirectionalMontageSet> DeathMontages;
	
};
