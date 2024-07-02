#pragma once

#include "CoreMinimal.h"
#include "Animation/AimOffsetBlendSpace.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundCue.h"
#include "WeaponData.generated.h"


USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Character")
	TObjectPtr<UAnimSequence> TPCharacterIdlePoseUB;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Character")
	TObjectPtr<UAnimSequence> TPCharacterCrouchPoseUB;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Character")
	TObjectPtr<UAimOffsetBlendSpace> TPCharacterAimOffsetUB;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Character")
	TObjectPtr<UAimOffsetBlendSpace> TPCharacterCrouchAimOffsetUB;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Character")
	TObjectPtr<UAnimMontage> TPCharacterEquipMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Character")
	TObjectPtr<UAnimMontage> TPCharacterFireMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Character")
	TObjectPtr<UAnimMontage> TPCharacterReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Weapon")
	TObjectPtr<UAnimMontage> TPWeaponEquipMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Weapon")
	TObjectPtr<UAnimMontage> TPWeaponFireMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Weapon")
	TObjectPtr<UAnimMontage> TPWeaponReloadMontage;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterIdlePose;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterIdleAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterWalkAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterRunAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterCrouchIn;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterCrouchOut;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterCrouchWalkAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterJumpAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterJumpLandAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAimOffsetBlendSpace> FPCharacterAimOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimMontage> FPCharacterEquipMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimMontage> FPCharacterFireMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimMontage> FPCharacterReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Weapon")
	TObjectPtr<UAnimMontage> FPWeaponEquipMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Weapon")
	TObjectPtr<UAnimMontage> FPWeaponFireMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Weapon")
	TObjectPtr<UAnimMontage> FPWeaponReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundCue> EquipSound;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundCue> ReloadSound;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundCue> FireSound;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundCue> AltFireSound;
	
};
