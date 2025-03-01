// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Action/NexusAction.h"
#include "Gunner/_Core/GunnerHitTypes.h"
#include "GunnerActionDamaged.generated.h"


class UGunnerHitMessageData;


/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerActionDamaged : public UNexusAction
{
	GENERATED_BODY()

public:
	UGunnerActionDamaged();
	virtual void OnTriggerAction() override;

	UFUNCTION(BlueprintCallable)
	EGunnerHitDirectionType GetHitDirectionType() const;
	UFUNCTION(BlueprintCallable)
	EGunnerHitBoneType GetHitBoneType(FName HitBoneName) const;
	UFUNCTION(BlueprintCallable)
	FString GetHitBoneTypeAsString(FName HitBoneName) const;
	UFUNCTION(BlueprintCallable)
	UAnimMontage* GetDesiredHitMontage(FName HitBoneName) const;
	UFUNCTION(BlueprintCallable)
	UAnimMontage* GetDesiredDeathMontage(FName HitBoneName, bool bLarge) const;

	UFUNCTION(BlueprintCallable)
	UGunnerHitMessageData* GetHitMessageData() const { return HitMessageData.Get(); }

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TMap<EGunnerHitBoneType, FGunnerDirectionalMontageSet> HitMontages;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TMap<EGunnerHitBoneType, FGunnerDirectionalMontageSet> DeathMontages;

	TArray<FName> HeadBoneNames;
	TArray<FName> LegBoneNames;

	UPROPERTY()
	TWeakObjectPtr<UGunnerHitMessageData> HitMessageData;

	mutable int32 MontageSetIndex = 0;
};
