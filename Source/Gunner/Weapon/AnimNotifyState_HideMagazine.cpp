// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_HideMagazine.h"

void UAnimNotifyState_HideMagazine::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	TArray<USceneComponent*> AttachedComponents = MeshComp->GetAttachChildren();
	for (USceneComponent* Component : AttachedComponents)
	{
		if (Component->GetAttachSocketName() != MagazineSocketName)
		{
			continue;
		}

		Magazine = Cast<UStaticMeshComponent>(Component);
		if (Magazine)
		{
			Magazine->SetVisibility(false);
		}
	}
}

void UAnimNotifyState_HideMagazine::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if(Magazine)
	{
		Magazine->SetVisibility(true);
	}
}
