// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NexusAnimMontagePlayerInterface.generated.h"

class UNexusAnimMontagePlayerComponent;
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UNexusAnimMontagePlayerInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

/**
 * 
 */
class NEXUSACTION_API INexusAnimMontagePlayerInterface
{
	GENERATED_IINTERFACE_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UNexusAnimMontagePlayerComponent* GetAnimMontagePlayer();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	USkeletalMeshComponent* GetFirstPersonMeshComponent() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	USkeletalMeshComponent* GetThirdPersonMeshComponent() const;
};
