// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GunnerAnimMontagePlayerInterface.generated.h"

class UGunnerAnimMontagePlayerComponent;
// This class does not need to be modified.
UINTERFACE()
class UGunnerAnimMontagePlayerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GUNNER_API IGunnerAnimMontagePlayerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UGunnerAnimMontagePlayerComponent* GetAnimMontagePlayer();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	USkeletalMeshComponent* GetFirstPersonMeshComponent() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	USkeletalMeshComponent* GetThirdPersonMeshComponent() const;
};
