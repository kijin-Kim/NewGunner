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

void AGunnerEquipment::OnAcquire()
{
	AActor* ActorOwner = GetOwner();
	check(ActorOwner);
	if (ActorOwner->HasAuthority())
	{
		AuthAddActionsOnAcquire();
	}
	AttachEquipmentToOwner();
}

void AGunnerEquipment::OnLost()
{
	AActor* ActorOwner = GetOwner();
	check(ActorOwner);
	if (ActorOwner->HasAuthority())
	{
		AuthRemoveActionsOnAcquire(ActorOwner);
	}
}

void AGunnerEquipment::AuthAddActionsOnEquip()
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
			AddedActionHandlesOnEquip.Add(ActionComponent->AuthAddAction(ActionDefinition));
		}
	}
}

void AGunnerEquipment::AuthRemoveActionsOnEquip()
{
	UGunnerActionComponent* ActionComponent = UGunnerActionComponent::GetActionComponentFromActor(GetOwner());
	check(ActionComponent);
	for (auto& ActionHandle : AddedActionHandlesOnEquip)
	{
		ActionComponent->AuthRemoveAction(ActionHandle);
	}
	AddedActionHandlesOnEquip.Empty();
}

void AGunnerEquipment::AuthAddActionsOnAcquire()
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
			AddedActionHandlesOnAcquire.Add(ActionComponent->AuthAddAction(ActionDefinition));
		}
	}
}

void AGunnerEquipment::AuthRemoveActionsOnAcquire(AActor* OldOwner)
{
	UGunnerActionComponent* ActionComponent = UGunnerActionComponent::GetActionComponentFromActor(OldOwner);
	check(ActionComponent);
	for (auto& ActionHandle : AddedActionHandlesOnAcquire)
	{
		ActionComponent->AuthRemoveAction(ActionHandle);
	}
	AddedActionHandlesOnAcquire.Empty();
}

void AGunnerEquipment::AttachEquipmentToOwner()
{
	AActor* ActorOwner = GetOwner();
	check(ActorOwner);
	if (ActorOwner->Implements<UAnimMontagePlayerInterface>())
	{
		USkeletalMeshComponent* CwnerFPMeshComponent = IAnimMontagePlayerInterface::Execute_GetFirstPersonMeshComponent(ActorOwner);
		USkeletalMeshComponent* OwnerTPMeshComponent = IAnimMontagePlayerInterface::Execute_GetThirdPersonMeshComponent(ActorOwner);

		AttachToComponent(OwnerTPMeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("CameraSocket"));
		FirstPersonMeshComponent->AttachToComponent(CwnerFPMeshComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("WeaponPoint"));
		ThirdPersonMeshComponent->AttachToComponent(OwnerTPMeshComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("WeaponPoint"));
	}
}

void AGunnerEquipment::OnEquipped()
{
	SetOwnerLocomotionAnimSet(LocomotionAnimSet);
	SetMeshVisibility(true);

	if (GetOwner()->HasAuthority())
	{
		AuthAddActionsOnEquip();
	}
}

void AGunnerEquipment::OnUnequipped()
{
	SetOwnerLocomotionAnimSet(nullptr);
	SetMeshVisibility(false);

	if (GetOwner()->HasAuthority())
	{
		AuthRemoveActionsOnEquip();
	}
}

void AGunnerEquipment::SetMeshVisibility(bool bVisible)
{
	FirstPersonMeshComponent->SetVisibility(bVisible);
	ThirdPersonMeshComponent->SetVisibility(bVisible);
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
