// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiPerspectiveMesh.h"


// Add default functionality here for any IMultiPerspectiveMesh functions that are not pure virtual.
float IMultiPerspectiveMesh::PlayMontage(UAnimMontage* AnimMontage, bool bIsThirdPerson, float InPlayRate, FName StartSectionName)
{
	USkeletalMeshComponent* TargetMesh = bIsThirdPerson ? GetThirdPersonMeshComponent() : GetFirstPersonMeshComponent();
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
