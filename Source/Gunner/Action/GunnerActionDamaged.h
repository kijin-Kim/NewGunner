// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NexusAction.h"
#include "GunnerActionDamaged.generated.h"


class UGunnerHitMessageData;

UENUM(BlueprintType)
enum class EGunnerHitDirectionType : uint8
{
	Front,
	Back,
	Left,
	Right,
};

UENUM(BlueprintType)
enum class EGunnerHitBoneType : uint8
{
	Head UMETA(DisplayName = "Head"),
	Body UMETA(DisplayName = "Body"),
	Leg UMETA(DisplayName = "Leg"),
};

USTRUCT(BlueprintType)
struct FGunnerDirectionalMontage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> Front;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> Back;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> Left;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> Right;
};

USTRUCT(BlueprintType)
struct FGunnerDirectionalMontageSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FGunnerDirectionalMontage> MontageSet;
};

/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerActionDamaged : public UNexusAction
{
	GENERATED_BODY()

public:
	UGunnerActionDamaged();
	virtual void OnTriggerAction_Implementation() override;

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
