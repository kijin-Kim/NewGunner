// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerEquippable.h"

#include "Animation/NexusAnimMontagePlayerComponent.h"
#include "Animation/NexusAnimMontagePlayerInterface.h"
#include "Gunner/_Core/GunnerTeamAgentInterface.h"
#include "Gunner/_Core/Animation/GunnerAnimInstance.h"


// Sets default values
AGunnerEquippable::AGunnerEquippable()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	DefaultSceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	SetRootComponent(DefaultSceneRootComponent);
	FirstPersonMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMeshComponent->SetupAttachment(GetRootComponent());
	FirstPersonMeshComponent->bOnlyOwnerSee = true;
	FirstPersonMeshComponent->CastShadow = false;
	FirstPersonMeshComponent->bRenderCustomDepth = true;

	AnimMontagePlayerComponent = CreateDefaultSubobject<UNexusAnimMontagePlayerComponent>(TEXT("AnimMontagePlayer"));
}

void AGunnerEquippable::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ThirdPersonMeshComponent = DuplicateObject(FirstPersonMeshComponent, this);
	ThirdPersonMeshComponent->Rename(TEXT("ThirdPersonMeshComponent"), this);
	ThirdPersonMeshComponent->bOnlyOwnerSee = false;
	ThirdPersonMeshComponent->CastShadow = true;
	ThirdPersonMeshComponent->bOwnerNoSee = true;
	ThirdPersonMeshComponent->bRenderCustomDepth = false;
	ThirdPersonMeshComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	ThirdPersonMeshComponent->RegisterComponent();

	TArray<USceneComponent*> FPChildren;
	FirstPersonMeshComponent->GetChildrenComponents(true, FPChildren);
	for (USceneComponent* Child : FPChildren)
	{
		if (UMeshComponent* ChildMesh = Cast<UMeshComponent>(Child))
		{
			ChildMesh->bRenderCustomDepth = true;
			UMeshComponent* NewChild = Cast<UMeshComponent>(DuplicateObject(ChildMesh, this));
			NewChild->bOnlyOwnerSee = false;
			NewChild->CastShadow = true;
			NewChild->bOwnerNoSee = true;
			NewChild->AttachToComponent(ThirdPersonMeshComponent, FAttachmentTransformRules::KeepRelativeTransform, ChildMesh->GetAttachSocketName());
			NewChild->RegisterComponent();
		}
	}
}

UNexusAnimMontagePlayerComponent* AGunnerEquippable::GetAnimMontagePlayer_Implementation()
{
	return AnimMontagePlayerComponent;
}

USkeletalMeshComponent* AGunnerEquippable::GetFirstPersonMeshComponent_Implementation() const
{
	return FirstPersonMeshComponent;
}

USkeletalMeshComponent* AGunnerEquippable::GetThirdPersonMeshComponent_Implementation() const
{
	return ThirdPersonMeshComponent;
}


void AGunnerEquippable::OnAcquired()
{
	Super::OnAcquired();
	AttachToOwner();
	SetMeshVisibility(false);
}

void AGunnerEquippable::OnActivated()
{
	Super::OnActivated();
	SetMeshVisibility(true);
	SetOwnerLocomotionAnimSet(LocomotionAnimSet);
	ActivateWallPenetration(true);
}

void AGunnerEquippable::OnDeactivated()
{
	Super::OnDeactivated();
	SetMeshVisibility(false);
	SetOwnerLocomotionAnimSet(nullptr);
	ActivateWallPenetration(false);
}

void AGunnerEquippable::AttachToOwner() const
{
	const AActor* ActorOwner = GetOwner();
	if (ActorOwner && ActorOwner->Implements<UNexusAnimMontagePlayerInterface>())
	{
		USkeletalMeshComponent* OwnerFPMeshComponent = INexusAnimMontagePlayerInterface::Execute_GetFirstPersonMeshComponent(ActorOwner);
		USkeletalMeshComponent* OwnerTPMeshComponent = INexusAnimMontagePlayerInterface::Execute_GetThirdPersonMeshComponent(ActorOwner);

		FirstPersonMeshComponent->AttachToComponent(OwnerFPMeshComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("Socket_MasterWeapon"));
		ThirdPersonMeshComponent->AttachToComponent(OwnerTPMeshComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("Socket_MasterWeapon"));
	}
}

void AGunnerEquippable::SetMeshVisibility(bool bVisible) const
{
	FirstPersonMeshComponent->SetVisibility(bVisible, true);
	ThirdPersonMeshComponent->SetVisibility(bVisible, true);
}


void AGunnerEquippable::SetOwnerLocomotionAnimSet(UGunnerLocomotionAnimSet* InLocomotionAnimSet) const
{
	AActor* ActorOwner = GetOwner();
	if (!ActorOwner || !ActorOwner->Implements<UNexusAnimMontagePlayerInterface>())
	{
		return;
	}

	USkeletalMeshComponent* OwnerFPMeshComponent = INexusAnimMontagePlayerInterface::Execute_GetFirstPersonMeshComponent(ActorOwner);
	USkeletalMeshComponent* OwnerTPMeshComponent = INexusAnimMontagePlayerInterface::Execute_GetThirdPersonMeshComponent(ActorOwner);

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

void AGunnerEquippable::ActivateWallPenetration(bool bActive) const
{
	if (!bActive)
	{
		FirstPersonMeshComponent->SetRenderCustomDepth(false);
		ThirdPersonMeshComponent->SetRenderCustomDepth(false);
		return;
	}
	
	ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	APlayerController* PlayerController = LocalPlayer->PlayerController;
	IGunnerTeamAgentInterface* TeamAgentInterface = PlayerController->GetPlayerState<IGunnerTeamAgentInterface>();
	ETeamAttitude::Type Attitude = TeamAgentInterface->GetTeamAttitudeTowards(*GetOwner());
	

	if (Attitude == ETeamAttitude::Friendly)
	{
		ThirdPersonMeshComponent->SetRenderCustomDepth(true);
		ThirdPersonMeshComponent->SetCustomDepthStencilValue(1);

		TArray<USceneComponent*> TPChildren;
		ThirdPersonMeshComponent->GetChildrenComponents(true, TPChildren);
		for (USceneComponent* Child : TPChildren)
		{
			if (UMeshComponent* MeshComponent = Cast<UMeshComponent>(Child))
			{
				MeshComponent->SetRenderCustomDepth(true);
				MeshComponent->SetCustomDepthStencilValue(1);
			}
		}
	}
	else
	{
		ThirdPersonMeshComponent->SetRenderCustomDepth(false);
		TArray<USceneComponent*> TPChildren;
		ThirdPersonMeshComponent->GetChildrenComponents(true, TPChildren);
		for (USceneComponent* Child : TPChildren)
		{
			if (UMeshComponent* MeshComponent = Cast<UMeshComponent>(Child))
			{
				MeshComponent->SetRenderCustomDepth(false);
			}
		}
	}
}
