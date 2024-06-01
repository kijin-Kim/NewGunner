#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponData.generated.h"

USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UAnimInstance> TPCharacterAnimInstance;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> TPCharacterEquipMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> TPWeaponEquipMontage;
};
