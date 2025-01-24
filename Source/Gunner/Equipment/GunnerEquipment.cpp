// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerEquipment.h"

#include "Engine/Canvas.h"
#include "Gunner/Animation/GunnerAnimInstance.h"
#include "Gunner/Animation/GunnerAnimMontagePlayerComponent.h"
#include "Gunner/_Core/ActionSystem/GunnerAction.h"
#include "Gunner/_Core/ActionSystem/GunnerActionComponent.h"


// Sets default values
AGunnerEquipment::AGunnerEquipment()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	DefaultSceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	SetRootComponent(DefaultSceneRootComponent);
	FirstPersonMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMeshComponent->SetupAttachment(GetRootComponent());
	FirstPersonMeshComponent->bOnlyOwnerSee = true;
	FirstPersonMeshComponent->CastShadow = false;

	AnimMontagePlayerComponent = CreateDefaultSubobject<UGunnerAnimMontagePlayerComponent>(TEXT("AnimMontagePlayer"));
}

void AGunnerEquipment::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ThirdPersonMeshComponent = DuplicateObject(FirstPersonMeshComponent, this);
	ThirdPersonMeshComponent->Rename(TEXT("ThirdPersonMeshComponent"), this);
	ThirdPersonMeshComponent->bOnlyOwnerSee = false;
	ThirdPersonMeshComponent->CastShadow = true;
	ThirdPersonMeshComponent->bOwnerNoSee = true;
	ThirdPersonMeshComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	ThirdPersonMeshComponent->RegisterComponent();

	TArray<USceneComponent*> FPChildren;
	FirstPersonMeshComponent->GetChildrenComponents(true, FPChildren);
	for (USceneComponent* Child : FPChildren)
	{
		if (Child->IsA<UPrimitiveComponent>())
		{
			UPrimitiveComponent* NewChild = Cast<UPrimitiveComponent>(DuplicateObject(Child, this));
			NewChild->bOnlyOwnerSee = false;
			NewChild->CastShadow = true;
			NewChild->bOwnerNoSee = true;
			NewChild->AttachToComponent(ThirdPersonMeshComponent, FAttachmentTransformRules::KeepRelativeTransform, Child->GetAttachSocketName());
			NewChild->RegisterComponent();
		}
	}
}

void AGunnerEquipment::AttachEquipmentToOwner()
{
	AActor* ActorOwner = GetOwner();
	if (ActorOwner && ActorOwner->Implements<UGunnerAnimMontagePlayerInterface>())
	{
		USkeletalMeshComponent* OwnerFPMeshComponent = IGunnerAnimMontagePlayerInterface::Execute_GetFirstPersonMeshComponent(ActorOwner);
		USkeletalMeshComponent* OwnerTPMeshComponent = IGunnerAnimMontagePlayerInterface::Execute_GetThirdPersonMeshComponent(ActorOwner);

		AttachToComponent(OwnerTPMeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("CameraSocket"));
		FirstPersonMeshComponent->AttachToComponent(OwnerFPMeshComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("WeaponPoint"));
		ThirdPersonMeshComponent->AttachToComponent(OwnerTPMeshComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("WeaponPoint"));
	}
}

void AGunnerEquipment::OnAuthAcquired()
{
	AActor* ActorOwner = GetOwner();
	check(ActorOwner);
	if (ActorOwner->HasAuthority())
	{
		AuthAddDesiredActions(ActionsToAddOnAcquired, AddedActionHandlesOnAcquired);
	}
	AttachEquipmentToOwner();
}

void AGunnerEquipment::OnAuthLost()
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
	AActor* ActorOwner = GetOwner();
	if (ActorOwner && ActorOwner->HasAuthority())
	{
		AuthAddDesiredActions(ActionsToAddOnEquip, AddedActionHandlesOnEquip);

		UGunnerActionComponent* ActionComponent = UGunnerActionComponent::GetActionComponentFromActor(ActorOwner);
		check(ActionComponent);

		UGunnerActionProperty* BulletProperty = ActionComponent->GetProperty(FGameplayTag::RequestGameplayTag(FName("Property.Weapon.Bullet")));
		BulletProperty->SetStaticValue(BulletCount);


		UGunnerActionProperty* MagazineBulletProperty = ActionComponent->GetProperty(FGameplayTag::RequestGameplayTag(FName("Property.Weapon.MagazineBullet")));
		MagazineBulletProperty->SetStaticValue(MagazineBulletCount);


		UGunnerActionProperty* MaxBulletPerMagazineProperty = ActionComponent->GetProperty(FGameplayTag::RequestGameplayTag(FName("Property.Weapon.MaxBulletPerMagazine")));
		MaxBulletPerMagazineProperty->SetStaticValue(MaxBulletPerMagazineCount);
	}
}

void AGunnerEquipment::OnUnequipped()
{
	SetOwnerLocomotionAnimSet(nullptr);
	SetMeshVisibility(false);

	AActor* ActorOwner = GetOwner();
	if (ActorOwner && ActorOwner->HasAuthority())
	{
		AuthRemoveDesiredActions(AddedActionHandlesOnEquip);

		UGunnerActionComponent* ActionComponent = UGunnerActionComponent::GetActionComponentFromActor(ActorOwner);
		check(ActionComponent);

		UGunnerActionProperty* BulletProperty = ActionComponent->GetProperty(FGameplayTag::RequestGameplayTag(FName("Property.Weapon.Bullet")));
		BulletCount = BulletProperty->GetStaticValue();

		UGunnerActionProperty* MagazineBulletProperty = ActionComponent->GetProperty(FGameplayTag::RequestGameplayTag(FName("Property.Weapon.MagazineBullet")));
		MagazineBulletCount = MagazineBulletProperty->GetStaticValue();

		UGunnerActionProperty* MaxBulletPerMagazineProperty = ActionComponent->GetProperty(FGameplayTag::RequestGameplayTag(FName("Property.Weapon.MaxBulletPerMagazine")));
		MaxBulletPerMagazineCount = MaxBulletPerMagazineProperty->GetStaticValue();
	}
}

void AGunnerEquipment::SetMeshVisibility(bool bVisible)
{
	FirstPersonMeshComponent->SetVisibility(bVisible, true);
	ThirdPersonMeshComponent->SetVisibility(bVisible, true);
}


void AGunnerEquipment::OnRep_Owner()
{
	Super::OnRep_Owner();
	AttachEquipmentToOwner();
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
			if (ActorOwner->HasAuthority())
			{
				AddedActionHandles.Add(ActionComponent->AuthAddAction(ActionDefinition));
			}
		}
	}
}

void AGunnerEquipment::AuthRemoveDesiredActions(TArray<FGunnerActionDefinitionHandle>& AddedActionHandles)
{
	AActor* ActorOwner = GetOwner();
	check(ActorOwner);
	if (!ActorOwner->HasAuthority())
	{
		return;
	}

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
	if (!ActorOwner || !ActorOwner->Implements<UGunnerAnimMontagePlayerInterface>())
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
		else if(AnimInstance->GetLocomotionAnimSet() == LocomotionAnimSet)
		{
			AnimInstance->ClearLocomotionAnimSet();
		}
	}
}

