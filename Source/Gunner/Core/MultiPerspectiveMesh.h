// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MultiPerspectiveMesh.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UMultiPerspectiveMesh : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GUNNER_API IMultiPerspectiveMesh
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual USkeletalMeshComponent* GetFirstPersonMeshComponent() const = 0;
	virtual USkeletalMeshComponent* GetThirdPersonMeshComponent() const = 0;
	float PlayMontage(UAnimMontage* AnimMontage, bool bIsThirdPerson, float InPlayRate = 1.f, FName StartSectionName = NAME_None);

};
