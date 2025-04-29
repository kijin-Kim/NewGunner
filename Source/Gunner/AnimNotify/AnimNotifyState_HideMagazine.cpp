// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_HideMagazine.h"

void UAnimNotifyState_HideMagazine::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (UStaticMeshComponent* Magazine = GetMagazineMeshComponent(MeshComp))
	{
		Magazine->SetVisibility(false);
	}
}

void UAnimNotifyState_HideMagazine::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (UStaticMeshComponent* Magazine = GetMagazineMeshComponent(MeshComp))
	{
		if (MeshComp->GetVisibleFlag())
		{
			Magazine->SetVisibility(true);
		}
	}
}

UStaticMeshComponent* UAnimNotifyState_HideMagazine::GetMagazineMeshComponent(USkeletalMeshComponent* MeshComp) const
{
	TArray<USceneComponent*> AttachedComponents = MeshComp->GetAttachChildren();
	for (USceneComponent* Component : AttachedComponents)
	{
		if (Component->GetAttachSocketName() != MagazineSocketName)
		{
			continue;
		}
		return Cast<UStaticMeshComponent>(Component);
	}
	checkNoEntry();
	return nullptr;
}
