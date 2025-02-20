// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GunnerEquipmentDataAsset.generated.h"

class UGunnerInstancedAnimSet;
class UGunnerDamageType;
struct FDamageContext;
class UGunnerLocomotionAnimSet;


/**
 * 
 */
UCLASS(BlueprintType)
class GUNNER_API UGunnerEquipmentDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DeterminesOutputType = "AnimSetClass"))
	UGunnerInstancedAnimSet* GetAnimSetByClass(TSubclassOf<UGunnerInstancedAnimSet> AnimSetClass) const;
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float CalculateDamageByContext(const FDamageContext& DamageContext) const;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UserInterface")
	TObjectPtr<UTexture2D> OriginalIcon;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UserInterface")
	TObjectPtr<UTexture2D> SimplifiedIcon;
	
	UPROPERTY(EditAnywhere, Instanced)
	TObjectPtr<const UGunnerDamageType> DamageType;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UGunnerLocomotionAnimSet> LocomotionAnimSet;
	UPROPERTY(EditAnywhere, Instanced, Category = "Animation")
	TArray<TObjectPtr<UGunnerInstancedAnimSet>> InstancedAnimSets;

	
};
