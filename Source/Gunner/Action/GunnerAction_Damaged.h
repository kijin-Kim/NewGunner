// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Action/NexusAction.h"
#include "Gunner/_Core/GunnerHitBoxInterface.h"
#include "Gunner/_Core/GunnerDirectionalMontage.h"
#include "Gunner/_Core/Damage/GunnerDamageType.h"
#include "GunnerAction_Damaged.generated.h"


class UGunnerDamageContext;


/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerAction_Damaged : public UNexusAction
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	UAnimMontage* GetDesiredHitMontage(FName HitBoneName) const;
	UFUNCTION(BlueprintCallable)
	UAnimMontage* GetDesiredDeathMontage(FName HitBoneName, bool bLarge) const;

	
protected:
	virtual void OnTriggerAction() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TMap<EGunnerHitBoxType, FGunnerDirectionalMontageSet> HitMontages;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TMap<EGunnerHitBoxType, FGunnerDirectionalMontageSet> DeathMontages;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGunnerDamageContext> DamageContext;
	
	mutable int32 MontageSetIndex = 0;
};
