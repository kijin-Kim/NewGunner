// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerEquipment.h"

#include "Engine/Canvas.h"
#include "Gunner/Core/GunnerAnimMontagePlayerComponent.h"
#include "Gunner/Core/GunnerAnimInstance.h"
#include "Gunner/Core/GunnerPropertyComponent.h"
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

	AnimMontagePlayerComponent = CreateDefaultSubobject<UGunnerAnimMontagePlayerComponent>(TEXT("AnimMontagePlayer"));
	SetMeshVisibility(false);

}

void AGunnerEquipment::AttachEquipmentToOwner()
{
	AActor* ActorOwner = GetOwner();
	check(ActorOwner);
	if (ActorOwner->Implements<UGunnerAnimMontagePlayerInterface>())
	{
		USkeletalMeshComponent* OwnerFPMeshComponent = IGunnerAnimMontagePlayerInterface::Execute_GetFirstPersonMeshComponent(ActorOwner);
		USkeletalMeshComponent* OwnerTPMeshComponent = IGunnerAnimMontagePlayerInterface::Execute_GetThirdPersonMeshComponent(ActorOwner);

		AttachToComponent(OwnerTPMeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("CameraSocket"));
		FirstPersonMeshComponent->AttachToComponent(OwnerFPMeshComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("WeaponPoint"));
		ThirdPersonMeshComponent->AttachToComponent(OwnerTPMeshComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("WeaponPoint"));
	}
}

void AGunnerEquipment::OnAcquired()
{
	AActor* ActorOwner = GetOwner();
	check(ActorOwner);
	if (ActorOwner->HasAuthority())
	{
		AuthAddDesiredActions(ActionsToAddOnAcquired, AddedActionHandlesOnAcquired);
	}
	AttachEquipmentToOwner();
}

void AGunnerEquipment::OnLost()
{
	AActor* ActorOwner = GetOwner();
	check(ActorOwner);
	if (ActorOwner->HasAuthority())
	{
		AuthRemoveDesiredActions(AddedActionHandlesOnAcquired);
	}
}

void AGunnerEquipment::OnEquipped()
{
	SetOwnerLocomotionAnimSet(LocomotionAnimSet);
	SetMeshVisibility(true);

	if (GetOwner()->HasAuthority())
	{
		AuthAddDesiredActions(ActionsToAddOnEquip, AddedActionHandlesOnEquip);
	}
}

void AGunnerEquipment::OnUnequipped()
{
	SetOwnerLocomotionAnimSet(nullptr);
	SetMeshVisibility(false);

	if (GetOwner()->HasAuthority())
	{
		AuthRemoveDesiredActions(AddedActionHandlesOnEquip);
	}
}


UGunnerAnimMontagePlayerComponent* AGunnerEquipment::GetAnimMontagePlayer_Implementation()
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

void AGunnerEquipment::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y)
{
	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;
	DisplayDebugManager.DrawString(FString::Printf(TEXT("Equipment: %s"), *GetName()));


	TArray<UGunnerPropertyComponent*> Components;
	GetComponents(UGunnerPropertyComponent::StaticClass(), Components);
	for (UGunnerPropertyComponent* Component : Components)
	{
		Component->OnShowDebugInfo(HUD, Canvas, DebugDisplayInfo, X, Y);
	}
}

void AGunnerEquipment::AuthAddDesiredActions(const TArray<TSubclassOf<UGunnerAction>>& ActionsToAdd, TArray<FGunnerActionDefinitionHandle>& AddedActionHandles)
{
	AActor* ActorOwner = GetOwner();
	check(ActorOwner);
	UGunnerActionComponent* ActionComponent = UGunnerActionComponent::GetActionComponentFromActor(ActorOwner);
	check(ActionComponent);
	for (auto ActionClass : ActionsToAdd)
	{
		if (ActionClass)
		{
			FGunnerActionDefinition ActionDefinition(this, ActionClass);
			AddedActionHandles.Add(ActionComponent->AuthAddAction(ActionDefinition));
		}
	}
}

void AGunnerEquipment::AuthRemoveDesiredActions(TArray<FGunnerActionDefinitionHandle>& AddedActionHandles)
{
	AActor* ActorOwner = GetOwner();
	check(ActorOwner);
	UGunnerActionComponent* ActionComponent = UGunnerActionComponent::GetActionComponentFromActor(ActorOwner);
	check(ActionComponent);
	for (auto& ActionHandle : AddedActionHandles)
	{
		ActionComponent->AuthRemoveAction(ActionHandle);
	}
	AddedActionHandles.Empty();
}

void AGunnerEquipment::SetOwnerLocomotionAnimSet(UGunnerLocomotionAnimSet* InLocomotionAnimSet)
{
	AActor* ActorOwner = GetOwner();
	check(ActorOwner);
	if (!ActorOwner->Implements<UGunnerAnimMontagePlayerInterface>())
	{
		return;
	}

	USkeletalMeshComponent* OwnerFPMeshComponent = IGunnerAnimMontagePlayerInterface::Execute_GetFirstPersonMeshComponent(ActorOwner);
	USkeletalMeshComponent* OwnerTPMeshComponent = IGunnerAnimMontagePlayerInterface::Execute_GetThirdPersonMeshComponent(ActorOwner);

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
		else
		{
			AnimInstance->ClearLocomotionAnimSet();
		}
	}
}

void AGunnerEquipment::SetMeshVisibility(bool bVisible)
{
	FirstPersonMeshComponent->SetVisibility(bVisible);
	ThirdPersonMeshComponent->SetVisibility(bVisible);
}
