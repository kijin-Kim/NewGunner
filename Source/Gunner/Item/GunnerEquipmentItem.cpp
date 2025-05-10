// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerEquipmentItem.h"

#include "Animation/NexusAnimMontagePlayerComponent.h"
#include "Animation/NexusAnimMontagePlayerInterface.h"
#include "Gunner/_Core/GunnerTeamAgentInterface.h"
#include "Gunner/_Core/Animation/GunnerAnimInstance.h"


// Sets default values
AGunnerEquipmentItem::AGunnerEquipmentItem()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	DefaultSceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	SetRootComponent(DefaultSceneRootComponent);
	FirstPersonMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMeshComponent->SetupAttachment(GetRootComponent());
	FirstPersonMeshComponent->CastShadow = false;
	FirstPersonMeshComponent->bRenderCustomDepth = true;
	FirstPersonMeshComponent->SetIsReplicated(false);

	AnimMontagePlayerComponent = CreateDefaultSubobject<UNexusAnimMontagePlayerComponent>(TEXT("AnimMontagePlayer"));
	AnimMontagePlayerComponent->SetIsReplicated(true);
}

void AGunnerEquipmentItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ThirdPersonMeshComponent = DuplicateObject(FirstPersonMeshComponent, this);
	ThirdPersonMeshComponent->Rename(TEXT("ThirdPersonMeshComponent"), this);
	ThirdPersonMeshComponent->CastShadow = true;
	ThirdPersonMeshComponent->bRenderCustomDepth = false;
	ThirdPersonMeshComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	ThirdPersonMeshComponent->RegisterComponent();
	ThirdPersonMeshComponent->SetIsReplicated(false);

	TArray<USceneComponent*> FPChildren;
	FirstPersonMeshComponent->GetChildrenComponents(true, FPChildren);
	for (USceneComponent* Child : FPChildren)
	{
		if (UMeshComponent* ChildMesh = Cast<UMeshComponent>(Child))
		{
			ChildMesh->bRenderCustomDepth = true;
			ChildMesh->SetIsReplicated(false);
			UMeshComponent* NewChild = Cast<UMeshComponent>(DuplicateObject(ChildMesh, this));
			NewChild->CastShadow = true;
			NewChild->AttachToComponent(ThirdPersonMeshComponent, FAttachmentTransformRules::KeepRelativeTransform, ChildMesh->GetAttachSocketName());
			NewChild->RegisterComponent();
			NewChild->SetIsReplicated(false);
		}
	}
}

UNexusAnimMontagePlayerComponent* AGunnerEquipmentItem::GetAnimMontagePlayer_Implementation()
{
	return AnimMontagePlayerComponent;
}

USkeletalMeshComponent* AGunnerEquipmentItem::GetFirstPersonMeshComponent_Implementation() const
{
	return FirstPersonMeshComponent;
}

USkeletalMeshComponent* AGunnerEquipmentItem::GetThirdPersonMeshComponent_Implementation() const
{
	return ThirdPersonMeshComponent;
}


void AGunnerEquipmentItem::OnAcquired(AActor* InAgentActor)
{
	Super::OnAcquired(InAgentActor);
	SetMeshVisibility(false);
	AttachToAgentActor();
	IGunnerTeamAgentInterface* TeamAgentInterface = Cast<IGunnerTeamAgentInterface>(AgentActor);
	check(TeamAgentInterface);
	if (ensure(TeamAgentInterface && TeamAgentInterface->GetOnTeamSetDelegate()))
	{
		TeamAgentInterface->GetOnTeamSetDelegate()->AddWeakLambda(this, [this](FGenericTeamId OldTeamId, FGenericTeamId NewTeamId)
		{
			SetCustomDepthStencilValue(NewTeamId + 1);
		});
	}


	SetCustomDepthStencilValue(TeamAgentInterface->GetGenericTeamId() + 1);
}

void AGunnerEquipmentItem::OnRemoved()
{
	SetMeshVisibility(false);

	IGunnerTeamAgentInterface* TeamAgentInterface = Cast<IGunnerTeamAgentInterface>(AgentActor);
	if (TeamAgentInterface && TeamAgentInterface->GetOnTeamSetDelegate())
	{
		TeamAgentInterface->GetOnTeamSetDelegate()->RemoveAll(this);
	}

	Super::OnRemoved();
}

void AGunnerEquipmentItem::OnActivated()
{
	Super::OnActivated();
	SetMeshVisibility(true);
	SetAgentActorLocomotionAnimSet(LocomotionAnimSet);
	SetRenderCustomDepth(true);
}

void AGunnerEquipmentItem::OnDeactivated()
{
	Super::OnDeactivated();
	SetMeshVisibility(false);
	SetAgentActorLocomotionAnimSet(nullptr);
	SetRenderCustomDepth(false);
}

void AGunnerEquipmentItem::AttachToAgentActor() const
{
	if (AgentActor && AgentActor->Implements<UNexusAnimMontagePlayerInterface>())
	{
		USkeletalMeshComponent* OwnerFPMeshComponent = INexusAnimMontagePlayerInterface::Execute_GetFirstPersonMeshComponent(AgentActor);
		USkeletalMeshComponent* OwnerTPMeshComponent = INexusAnimMontagePlayerInterface::Execute_GetThirdPersonMeshComponent(AgentActor);

		FirstPersonMeshComponent->AttachToComponent(OwnerFPMeshComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("Socket_MasterWeapon"));
		ThirdPersonMeshComponent->AttachToComponent(OwnerTPMeshComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("Socket_MasterWeapon"));
	}
}

void AGunnerEquipmentItem::SetMeshVisibility(bool bVisible) const
{
	APawn* AgentPawn = Cast<APawn>(AgentActor);
	if (bVisible)
	{
		if (AgentPawn->IsLocallyControlled())
		{
			FirstPersonMeshComponent->SetVisibility(true, true);
			ThirdPersonMeshComponent->SetVisibility(false, true);
		}
		else
		{
			FirstPersonMeshComponent->SetVisibility(false, true);
			ThirdPersonMeshComponent->SetVisibility(true, true);
		}
	}
	else
	{
		FirstPersonMeshComponent->SetVisibility(false, true);
		ThirdPersonMeshComponent->SetVisibility(false, true);
	}
}


void AGunnerEquipmentItem::SetAgentActorLocomotionAnimSet(UGunnerLocomotionAnimSet* InLocomotionAnimSet) const
{
	if (!AgentActor || !AgentActor->Implements<UNexusAnimMontagePlayerInterface>())
	{
		return;
	}

	USkeletalMeshComponent* OwnerFPMeshComponent = INexusAnimMontagePlayerInterface::Execute_GetFirstPersonMeshComponent(AgentActor);
	USkeletalMeshComponent* OwnerTPMeshComponent = INexusAnimMontagePlayerInterface::Execute_GetThirdPersonMeshComponent(AgentActor);

	TArray<UGunnerAnimInstance*> AnimInstances = {
		Cast<UGunnerAnimInstance>(OwnerFPMeshComponent->GetAnimInstance()),
		Cast<UGunnerAnimInstance>(OwnerTPMeshComponent->GetAnimInstance())
	};

	for (UGunnerAnimInstance* AnimInstance : AnimInstances)
	{
		if (!AnimInstance)
		{
			continue;
		}

		if (InLocomotionAnimSet)
		{
			AnimInstance->SetLocomotionAnimSet(InLocomotionAnimSet);
		}
		else if (AnimInstance->GetLocomotionAnimSet() == LocomotionAnimSet)
		{
			AnimInstance->ClearLocomotionAnimSet();
		}
	}
}

void AGunnerEquipmentItem::SetRenderCustomDepth(bool bSetRenderCustomDepth)
{
	ThirdPersonMeshComponent->SetRenderCustomDepth(bSetRenderCustomDepth);
	TArray<USceneComponent*> TPChildren;
	ThirdPersonMeshComponent->GetChildrenComponents(true, TPChildren);
	for (USceneComponent* Child : TPChildren)
	{
		if (UMeshComponent* MeshComponent = Cast<UMeshComponent>(Child))
		{
			MeshComponent->SetRenderCustomDepth(bSetRenderCustomDepth);
		}
	}
}

void AGunnerEquipmentItem::SetCustomDepthStencilValue(int32 StencilValue) const
{
	ThirdPersonMeshComponent->SetCustomDepthStencilValue(StencilValue);

	TArray<USceneComponent*> TPChildren;
	ThirdPersonMeshComponent->GetChildrenComponents(true, TPChildren);
	for (USceneComponent* Child : TPChildren)
	{
		if (UMeshComponent* MeshComponent = Cast<UMeshComponent>(Child))
		{
			ThirdPersonMeshComponent->SetCustomDepthStencilValue(StencilValue);
		}
	}
}
