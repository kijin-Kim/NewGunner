// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerEquipment.h"

#include "Gunner/Core/AnimMontagePlayerComponent.h"
#include "Gunner/Core/GunnerAnimInstance.h"
#include "Gunner/Core/ActionSystem/GunnerAction.h"
#include "Gunner/Core/ActionSystem/GunnerActionComponent.h"


// Sets default values
AGunnerEquipment::AGunnerEquipment()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	DefaultSceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	SetRootComponent(DefaultSceneRootComponent);
	FirstPersonMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMeshComponent->SetupAttachment(GetRootComponent());
	FirstPersonMeshComponent->SetIsReplicated(true);
	ThirdPersonMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ThirdPersonMesh"));
	ThirdPersonMeshComponent->SetupAttachment(GetRootComponent());
	ThirdPersonMeshComponent->SetIsReplicated(true);

	FirstPersonMeshComponent->bOnlyOwnerSee = true;
	ThirdPersonMeshComponent->bOwnerNoSee = true;

	AnimMontagePlayerComponent = CreateDefaultSubobject<UAnimMontagePlayerComponent>(TEXT("AnimMontagePlayer"));
}

void AGunnerEquipment::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);
	AttachEquipmentToOwner();
}

void AGunnerEquipment::AddActionsOnEquip()
{
	AActor* ActorOwner = GetOwner();
	check(ActorOwner);

	UGunnerActionComponent* ActionComponent = UGunnerActionComponent::GetActionComponentFromActor(GetOwner());
	check(ActionComponent);
	for (auto ActionClass : ActionsToAddOnEquip)
	{
		if (ActionClass)
		{
			FGunnerActionDefinition ActionDefinition(this, ActionClass);
			AddedActionHandlesOnEquip.Add(ActionComponent->AddAction(ActionDefinition));
		}
	}
}

void AGunnerEquipment::RemoveActionsOnEquip()
{
	UGunnerActionComponent* ActionComponent = UGunnerActionComponent::GetActionComponentFromActor(GetOwner());
	check(ActionComponent);
	for (auto& ActionHandle : AddedActionHandlesOnEquip)
	{
		ActionComponent->RemoveAction(ActionHandle);
	}
	AddedActionHandlesOnEquip.Empty();
}

void AGunnerEquipment::AddActionsOnAcquire()
{
	AActor* ActorOwner = GetOwner();
	check(ActorOwner);
	UGunnerActionComponent* ActionComponent = UGunnerActionComponent::GetActionComponentFromActor(ActorOwner);
	check(ActionComponent);
	for (auto ActionClass : ActionsToAddOnAcquire)
	{
		if (ActionClass)
		{
			FGunnerActionDefinition ActionDefinition(this, ActionClass);
			AddedActionHandlesOnAcquire.Add(ActionComponent->AddAction(ActionDefinition));
		}
	}
}

void AGunnerEquipment::RemoveActionsOnAcquire(AActor* OldOwner)
{
	UGunnerActionComponent* ActionComponent = UGunnerActionComponent::GetActionComponentFromActor(OldOwner);
	check(ActionComponent);
	for (auto& ActionHandle : AddedActionHandlesOnAcquire)
	{
		ActionComponent->RemoveAction(ActionHandle);
	}
	AddedActionHandlesOnAcquire.Empty();
}

void AGunnerEquipment::AttachEquipmentToOwner()
{
	AActor* ActorOwner = GetOwner();
	if (ActorOwner && ActorOwner->Implements<UAnimMontagePlayerInterface>())
	{
		USkeletalMeshComponent* CwnerFPMeshComponent = IAnimMontagePlayerInterface::Execute_GetFirstPersonMeshComponent(ActorOwner);
		USkeletalMeshComponent* OwnerTPMeshComponent = IAnimMontagePlayerInterface::Execute_GetThirdPersonMeshComponent(ActorOwner);

		AttachToComponent(OwnerTPMeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("CameraSocket"));
		FirstPersonMeshComponent->AttachToComponent(CwnerFPMeshComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("WeaponPoint"));
		ThirdPersonMeshComponent->AttachToComponent(OwnerTPMeshComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("WeaponPoint"));
		FirstPersonMeshComponent->SetVisibility(false);
		ThirdPersonMeshComponent->SetVisibility(false);
	}
}

void AGunnerEquipment::OnEquipped()
{
	SetOwnerLocomotionAnimSet(LocomotionAnimSet);
	FirstPersonMeshComponent->SetVisibility(true);
	ThirdPersonMeshComponent->SetVisibility(true);
}

void AGunnerEquipment::OnUnequipped()
{
	SetOwnerLocomotionAnimSet(nullptr);
	FirstPersonMeshComponent->SetVisibility(false);
	ThirdPersonMeshComponent->SetVisibility(false);
}

UAnimMontagePlayerComponent* AGunnerEquipment::GetAnimMontagePlayer_Implementation()
{
	return AnimMontagePlayerComponent;
}

USkeletalMeshComponent* AGunnerEquipment::GetFirstPersonMeshComponent_Implementation() const
{
	return FirstPersonMeshComponent;
}

USkeletalMeshComponent* AGunnerEquipment::GetThirdPersonMeshComponent_Implementation() const
{
	return ThirdPersonMeshComponent;
}

void AGunnerEquipment::SetOwnerLocomotionAnimSet(UGunnerLocomotionAnimSet* InLocomotionAnimSet)
{
	AActor* ActorOwner = GetOwner();
	if (!ActorOwner || !ActorOwner->Implements<UAnimMontagePlayerInterface>())
	{
		return;
	}

	USkeletalMeshComponent* CwnerFPMeshComponent = IAnimMontagePlayerInterface::Execute_GetFirstPersonMeshComponent(ActorOwner);
	USkeletalMeshComponent* OwnerTPMeshComponent = IAnimMontagePlayerInterface::Execute_GetThirdPersonMeshComponent(ActorOwner);

	TArray<UGunnerAnimInstance*> AnimInstances = {
		Cast<UGunnerAnimInstance>(CwnerFPMeshComponent->GetAnimInstance()),
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
		else
		{
			AnimInstance->ClearLocomotionAnimSet();
		}
	}
}

