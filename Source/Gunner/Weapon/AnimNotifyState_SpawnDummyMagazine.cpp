// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_SpawnDummyMagazine.h"

#include "Engine/StaticMeshActor.h"

void UAnimNotifyState_SpawnDummyMagazine::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	FTransform MagazineSocketTransform = MeshComp->GetSocketTransform(SocketName);

	DummyMagazine = MeshComp->GetWorld()->SpawnActorDeferred<AStaticMeshActor>(AStaticMeshActor::StaticClass(), MagazineSocketTransform);
	DummyMagazine->SetOwner(MeshComp->GetOwner());
	DummyMagazine->SetMobility(EComponentMobility::Type::Movable);
	DummyMagazine->GetStaticMeshComponent()->SetSimulatePhysics(true);
	DummyMagazine->GetStaticMeshComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	DummyMagazine->GetStaticMeshComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	DummyMagazine->GetStaticMeshComponent()->SetOnlyOwnerSee(MeshComp->bOnlyOwnerSee);
	DummyMagazine->GetStaticMeshComponent()->SetOwnerNoSee(MeshComp->bOwnerNoSee);
	DummyMagazine->SetLifeSpan(Animation->GetPlayLength());


	TArray<USceneComponent*> AttachedComponents;
	UStaticMesh* StaticMesh = nullptr;
	TArray<UMaterialInterface*> Materials;
	MeshComp->GetChildrenComponents(true, AttachedComponents);
	for (USceneComponent* Component : AttachedComponents)
	{
		UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Component);
		if (StaticMeshComponent && Component->GetAttachSocketName() == SocketName)
		{
			StaticMesh = StaticMeshComponent->GetStaticMesh();
			Materials = StaticMeshComponent->GetMaterials();
			break;
		}
	}

	DummyMagazine->GetStaticMeshComponent()->SetStaticMesh(StaticMesh);
	for (int32 i = 0; i < Materials.Num(); i++)
	{
		if (Materials[i])
		{
			DummyMagazine->GetStaticMeshComponent()->SetMaterial(i, Materials[i]);
		}
	}

	DummyMagazine->FinishSpawning(MagazineSocketTransform, true);

}

void UAnimNotifyState_SpawnDummyMagazine::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (DummyMagazine)
	{
		DummyMagazine->Destroy();
	}
}
