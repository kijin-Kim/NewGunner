// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Action/NexusAction.h"
#include "Gunner/_Core/GunnerHitBoxInterface.h"
#include "Gunner/_Core/GunnerDirectionalMontage.h"
#include "GunnerAction_Damaged.generated.h"


struct FGunnerDamageContext;


/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerAction_Damaged : public UNexusAction
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	UAnimMontage* GetDesiredHitMontage() const;


protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<EGunnerHitPartType, FGunnerDirectionalMontageSet> HitMontages;

private:
	mutable int32 MontageSetIndex = 0;
};
