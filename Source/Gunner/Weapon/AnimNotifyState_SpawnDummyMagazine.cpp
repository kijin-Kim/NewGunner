// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_SpawnDummyMagazine.h"

#include "Engine/StaticMeshActor.h"

void UAnimNotifyState_SpawnDummyMagazine::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	FTransform MagazineSocketTransform = MeshComp->GetSocketTransform(SocketName);
	DummyMagazine = MeshComp->GetWorld()->SpawnActor<AStaticMeshActor>(MagazineSocketTransform.GetLocation(), MagazineSocketTransform.GetRotation().Rotator());
	DummyMagazine->SetOwner(MeshComp->GetOwner());
	DummyMagazine->SetMobility(EComponentMobility::Type::Movable);
	DummyMagazine->GetStaticMeshComponent()->SetStaticMesh(StaticMesh);
	DummyMagazine->GetStaticMeshComponent()->SetMaterial(0, Material);
	DummyMagazine->GetStaticMeshComponent()->SetSimulatePhysics(true);
	DummyMagazine->GetStaticMeshComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	DummyMagazine->GetStaticMeshComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	DummyMagazine->GetStaticMeshComponent()->SetOnlyOwnerSee(MeshComp->bOnlyOwnerSee);
	DummyMagazine->GetStaticMeshComponent()->SetOwnerNoSee(MeshComp->bOwnerNoSee);
	DummyMagazine->SetLifeSpan(Animation->GetPlayLength());
}

void UAnimNotifyState_SpawnDummyMagazine::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);
	if(DummyMagazine)
	{
		DummyMagazine->Destroy();
	}
}
