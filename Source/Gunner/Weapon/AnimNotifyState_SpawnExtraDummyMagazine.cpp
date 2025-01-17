// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_SpawnExtraDummyMagazine.h"

#include "Engine/StaticMeshActor.h"

void UAnimNotifyState_SpawnExtraDummyMagazine::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	if (ExtraDummyMagazine)
	{
		ExtraDummyMagazine->Destroy();
	}

	ExtraDummyMagazine = MeshComp->GetWorld()->SpawnActor<AStaticMeshActor>();
	ExtraDummyMagazine->SetMobility(EComponentMobility::Type::Movable);
	ExtraDummyMagazine->GetStaticMeshComponent()->SetStaticMesh(StaticMesh);
	ExtraDummyMagazine->GetStaticMeshComponent()->SetMaterial(0, Material);
	ExtraDummyMagazine->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	ExtraDummyMagazine->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
	ExtraDummyMagazine->SetOwner(MeshComp->GetOwner());
	ExtraDummyMagazine->GetStaticMeshComponent()->SetOnlyOwnerSee(MeshComp->bOnlyOwnerSee);
	ExtraDummyMagazine->GetStaticMeshComponent()->SetOwnerNoSee(MeshComp->bOwnerNoSee);
	ExtraDummyMagazine->SetLifeSpan(Animation->GetPlayLength());
	
}

void UAnimNotifyState_SpawnExtraDummyMagazine::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (ExtraDummyMagazine)
	{
		ExtraDummyMagazine->Destroy();
	}
}
