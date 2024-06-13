// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimMontagePlayerComponent.h"

#include "AnimMontagePlayerInterface.h"


UAnimMontagePlayerComponent::UAnimMontagePlayerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UAnimMontagePlayerComponent::BeginPlay()
{
	Super::BeginPlay();
	check(GetOwner()->Implements<UAnimMontagePlayerInterface>() && "Owner Must Implements AnimMontagePlayerInterface");
}

float UAnimMontagePlayerComponent::PlayMontage(UAnimMontage* AnimMontage, bool bIsThirdPerson, float InPlayRate, FName StartSectionName)
{
	return LocalPlayMontage(AnimMontage, bIsThirdPerson, InPlayRate, StartSectionName);
}

float UAnimMontagePlayerComponent::LocalPlayMontage(UAnimMontage* AnimMontage, bool bIsThirdPerson, float InPlayRate, FName StartSectionName)
{
	USkeletalMeshComponent* TargetMesh = bIsThirdPerson
		                                     ? IAnimMontagePlayerInterface::Execute_GetThirdPersonMeshComponent(GetOwner())
		                                     : IAnimMontagePlayerInterface::Execute_GetFirstPersonMeshComponent(GetOwner());
	UAnimInstance* AnimInstance = (TargetMesh) ? TargetMesh->GetAnimInstance() : nullptr;
	if (AnimMontage && AnimInstance)
	{
		float const Duration = AnimInstance->Montage_Play(AnimMontage, InPlayRate);

		if (Duration > 0.f)
		{
			// Start at a given Section.
			if (StartSectionName != NAME_None)
			{
				AnimInstance->Montage_JumpToSection(StartSectionName, AnimMontage);
			}

			return Duration;
		}
	}

	return 0.f;
}
