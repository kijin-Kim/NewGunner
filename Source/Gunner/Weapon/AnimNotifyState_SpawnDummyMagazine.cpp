// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_SpawnDummyMagazine.h"

#include "Engine/StaticMeshActor.h"

void UAnimNotifyState_SpawnDummyMagazine::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (DummyMagazine)
	{
		DummyMagazine->Destroy();
	}
	
	FTransform MagazineSocketTransform = MeshComp->GetSocketTransform(TEXT("Magazine_Main"));
	DummyMagazine = MeshComp->GetWorld()->SpawnActor<AStaticMeshActor>(MagazineSocketTransform.GetLocation(), MagazineSocketTransform.GetRotation().Rotator());
	DummyMagazine->SetMobility(EComponentMobility::Type::Movable);
	DummyMagazine->GetStaticMeshComponent()->SetStaticMesh(StaticMesh);
	DummyMagazine->GetStaticMeshComponent()->SetMaterial(0, Material);
	DummyMagazine->GetStaticMeshComponent()->SetSimulatePhysics(true);
	DummyMagazine->GetStaticMeshComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	DummyMagazine->GetStaticMeshComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	DummyMagazine->SetOwner(MeshComp->GetOwner());
	DummyMagazine->GetStaticMeshComponent()->SetOnlyOwnerSee(MeshComp->bOnlyOwnerSee);
	DummyMagazine->GetStaticMeshComponent()->SetOwnerNoSee(MeshComp->bOwnerNoSee);
}

void UAnimNotifyState_SpawnDummyMagazine::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	
	if (DummyMagazine)
	{
		DummyMagazine->Destroy();
	}
}
