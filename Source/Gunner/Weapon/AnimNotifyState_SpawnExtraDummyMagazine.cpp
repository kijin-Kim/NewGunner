// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_SpawnExtraDummyMagazine.h"

#include "Engine/StaticMeshActor.h"

void UAnimNotifyState_SpawnExtraDummyMagazine::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	ExtraDummyMagazine = MeshComp->GetWorld()->SpawnActorDeferred<AStaticMeshActor>(AStaticMeshActor::StaticClass(), FTransform::Identity);
	ExtraDummyMagazine->GetStaticMeshComponent()->bOnlyOwnerSee = MeshComp->bOnlyOwnerSee;
	ExtraDummyMagazine->GetStaticMeshComponent()->bOwnerNoSee = MeshComp->bOwnerNoSee;
	ExtraDummyMagazine->GetStaticMeshComponent()->CastShadow = 0;
	ExtraDummyMagazine->SetOwner(MeshComp->GetOwner());
	ExtraDummyMagazine->SetMobility(EComponentMobility::Type::Movable);
	ExtraDummyMagazine->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
	ExtraDummyMagazine->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	
	ExtraDummyMagazine->SetLifeSpan(Animation->GetPlayLength());

	TArray<USceneComponent*> AttachedComponents;
	UStaticMesh* StaticMesh = nullptr;
	TArray<UMaterialInterface*> Materials;
	MeshComp->GetChildrenComponents(true, AttachedComponents);
	for (USceneComponent* Component : AttachedComponents)
	{
		UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Component);
		if (StaticMeshComponent && Component->GetAttachSocketName() == SourceMeshSocketName)
		{
			StaticMesh = StaticMeshComponent->GetStaticMesh();
			Materials = StaticMeshComponent->GetMaterials();
			break;
		}
	}
	ExtraDummyMagazine->GetStaticMeshComponent()->SetStaticMesh(StaticMesh);
	for (int32 i = 0; i < Materials.Num(); i++)
	{
		if (Materials[i])
		{
			ExtraDummyMagazine->GetStaticMeshComponent()->SetMaterial(i, Materials[i]);
		}
	}
	ExtraDummyMagazine->FinishSpawning(FTransform::Identity);
}