// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponData.generated.h"

class UAimOffsetBlendSpace;
/**
 * 
 */
UCLASS()
class GUNNER_API UWeaponData : public UDataAsset
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	FName FPWeaponSocketName = TEXT("R_WeaponPointSocket");
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	FName TPWeaponSocketName = TEXT("WeaponPoint");

	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Weapon")
	TObjectPtr<UAnimMontage> FPWeaponEquipMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TSubclassOf<UAnimInstance> FPCharacterAnimInstance;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimMontage> FPCharacterEquipMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Weapon")
	TObjectPtr<UAnimMontage> TPWeaponEquipMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Character")
	TSubclassOf<UAnimInstance> TPCharacterAnimInstance;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Character")
	TObjectPtr<UAnimMontage> TPCharacterEquipMontage;
	
};
