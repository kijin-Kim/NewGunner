#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponData.generated.h"

class UAimOffsetBlendSpace;

USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ThirdPerson|Character")
	TObjectPtr<UAnimSequence> TPCharacterIdlePoseUB;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ThirdPerson|Character")
	TObjectPtr<UAnimSequence> TPCharacterCrouchPoseUB;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ThirdPerson|Character")
	TObjectPtr<UAimOffsetBlendSpace> TPCharacterAimOffsetUB;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ThirdPerson|Character")
	TObjectPtr<UAimOffsetBlendSpace> TPCharacterCrouchAimOffsetUB;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ThirdPerson|Character")
	TObjectPtr<UAnimMontage> TPCharacterEquipMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ThirdPerson|Character")
	TObjectPtr<UAnimMontage> TPCharacterFireMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ThirdPerson|Character")
	TObjectPtr<UAnimMontage> TPCharacterReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ThirdPerson|Weapon")
	TObjectPtr<UAnimMontage> TPWeaponEquipMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ThirdPerson|Weapon")
	TObjectPtr<UAnimMontage> TPWeaponFireMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ThirdPerson|Weapon")
	TObjectPtr<UAnimMontage> TPWeaponReloadMontage;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterIdlePose;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterIdleAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterWalkAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterRunAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterCrouchIn;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterCrouchOut;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterJumpAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterJumpLandAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FirstPerson|Character")
	TObjectPtr<UAimOffsetBlendSpace> FPCharacterAimOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FirstPerson|Character")
	TObjectPtr<UAnimMontage> FPCharacterEquipMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FirstPerson|Character")
	TObjectPtr<UAnimMontage> FPCharacterFireMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FirstPerson|Character")
	TObjectPtr<UAnimMontage> FPCharacterReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FirstPerson|Weapon")
	TObjectPtr<UAnimMontage> FPWeaponEquipMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FirstPerson|Weapon")
	TObjectPtr<UAnimMontage> FPWeaponFireMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FirstPerson|Weapon")
	TObjectPtr<UAnimMontage> FPWeaponReloadMontage;
};
