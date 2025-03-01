// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GunnerEquipmentDef.generated.h"

class UNexusAction;
class AGunnerEquipment;
class UGunnerInstancedAnimSet;
class UGunnerDamageType;
struct FDamageContext;
class UGunnerLocomotionAnimSet;

USTRUCT(BlueprintType)
struct FGunnerEquipmentMontageSet
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> FirstPersonCharacterMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> ThirdPersonCharacterMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> FirstPersonEquipmentMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> ThirdPersonEquipmentMontage;
};

UCLASS(BlueprintType)
class GUNNER_API UGunnerEquipmentAnimSet : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UGunnerLocomotionAnimSet> LocomotionAnimSet;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	FGunnerEquipmentMontageSet EquipMontageSet;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	FGunnerEquipmentMontageSet ReloadMontageSet;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	FGunnerEquipmentMontageSet FireMontageSet;
};

/**
 * 
 */
UCLASS(BlueprintType)
class GUNNER_API UGunnerEquipmentDef : public UDataAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float CalculateDamageByContext(const FDamageContext& DamageContext) const;
	UFUNCTION(BlueprintCallable, meta = (DeterminesOutputType = "AnimSetClass"))
	UGunnerInstancedAnimSet* FindInstancedAnimSetByClass(TSubclassOf<UGunnerInstancedAnimSet> AnimSetClass) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Data")
	TSubclassOf<AGunnerEquipment> EquipmentClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UserInterface")
	FName EquipmentName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UserInterface")
	TObjectPtr<UTexture2D> StandardIconTexture;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UserInterface")
	TObjectPtr<UTexture2D> SimplifiedIconTexture;


	UPROPERTY(EditAnywhere, Instanced, Category = "Damage")
	TObjectPtr<const UGunnerDamageType> DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default")
	float FirePerSec;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default", meta = (Units = "s"))
	float EquipSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default", meta = (Units = "s"))
	float ReloadSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default")
	int32 Magazine;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default")
	int32 Reserve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
	TArray<TSubclassOf<UNexusAction>> ActionsToAddOnAcquired;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
	TArray<TSubclassOf<UNexusAction>> ActionsToAddOnEquipped;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UGunnerLocomotionAnimSet> LocomotionAnimSet;
	
	UPROPERTY(EditAnywhere, Instanced, Category = "Animation")
	TArray<TObjectPtr<UGunnerInstancedAnimSet>> InstancedAnimSets;
};
