// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerEquipment.h"

#include "GunnerEquipmentDef.h"
#include "Action/NexusAction.h"
#include "Engine/Canvas.h"
#include "Gunner/_Core/Animation/GunnerAnimInstance.h"
#include "Animation/NexusAnimMontagePlayerComponent.h"
#include "Gunner/_Core/GunnerTeamAgentInterface.h"
#include "NexusActionComponent.h"
#include "Gunner/Gunner.h"
#include "Net/UnrealNetwork.h"


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
	FirstPersonMeshComponent->bRenderCustomDepth = true;

	AnimMontagePlayerComponent = CreateDefaultSubobject<UNexusAnimMontagePlayerComponent>(TEXT("AnimMontagePlayer"));
}

void AGunnerEquipment::OnConstruction(const FTransform& Transform)
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

void AGunnerEquipment::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AGunnerEquipment, EquipmentDef, COND_InitialOnly);
}

void AGunnerEquipment::AttachEquipmentToOwner()
{
	AActor* ActorOwner = GetOwner();
	if (ActorOwner && ActorOwner->Implements<UNexusAnimMontagePlayerInterface>())
	{
		USkeletalMeshComponent* OwnerFPMeshComponent = INexusAnimMontagePlayerInterface::Execute_GetFirstPersonMeshComponent(ActorOwner);
		USkeletalMeshComponent* OwnerTPMeshComponent = INexusAnimMontagePlayerInterface::Execute_GetThirdPersonMeshComponent(ActorOwner);

		
		FirstPersonMeshComponent->AttachToComponent(OwnerFPMeshComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("Socket_MasterWeapon"));
		ThirdPersonMeshComponent->AttachToComponent(OwnerTPMeshComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("Socket_MasterWeapon"));
	}

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		if (IGunnerTeamAgentInterface* TeamAgentOwner = Cast<IGunnerTeamAgentInterface>(GetOwner()))
		{
			TeamAgentOwner->GetOnTeamSetDelegate()->AddUObject(this, &AGunnerEquipment::OnTeamSetEvent);
			OnTeamSetEvent(TeamAgentOwner->GetGenericTeamId(), TeamAgentOwner->GetGenericTeamId());
		}
	}, 1.0f, false);
}

void AGunnerEquipment::OnAuthAcquired()
{
	APawn* PawnOwner = GetOwner<APawn>();
	check(PawnOwner);
	if (PawnOwner->HasAuthority() && PawnOwner->IsLocallyControlled())
	{
		AuthAddDesiredActions(EquipmentDef->ActionsToAddOnAcquired, AddedActionHandlesOnAcquired);
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
		AuthAddDesiredActions(EquipmentDef->ActionsToAddOnEquipped, AddedActionHandlesOnEquip);

		UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(ActorOwner);
		check(ActionComponent);

		UNexusProperty* BulletProperty = ActionComponent->GetProperty(FGameplayTag::RequestGameplayTag(FName("Property.Weapon.Bullet")));
		BulletProperty->SetStaticValue(BulletCount);


		UNexusProperty* MagazineBulletProperty = ActionComponent->GetProperty(FGameplayTag::RequestGameplayTag(FName("Property.Weapon.MagazineBullet")));
		MagazineBulletProperty->SetStaticValue(MagazineBulletCount);


		UNexusProperty* MaxBulletPerMagazineProperty = ActionComponent->GetProperty(FGameplayTag::RequestGameplayTag(FName("Property.Weapon.MaxBulletPerMagazine")));
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

		UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(ActorOwner);
		check(ActionComponent);

		UNexusProperty* BulletProperty = ActionComponent->GetProperty(FGameplayTag::RequestGameplayTag(FName("Property.Weapon.Bullet")));
		BulletCount = BulletProperty->GetStaticValue();

		UNexusProperty* MagazineBulletProperty = ActionComponent->GetProperty(FGameplayTag::RequestGameplayTag(FName("Property.Weapon.MagazineBullet")));
		MagazineBulletCount = MagazineBulletProperty->GetStaticValue();

		UNexusProperty* MaxBulletPerMagazineProperty = ActionComponent->GetProperty(FGameplayTag::RequestGameplayTag(FName("Property.Weapon.MaxBulletPerMagazine")));
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
	if (GetNetConnection())
	{
		ServerAckClientAcquired();
	}
}

void AGunnerEquipment::ServerAckClientAcquired_Implementation()
{
	AuthAddDesiredActions(EquipmentDef->ActionsToAddOnAcquired, AddedActionHandlesOnAcquired);
}

UNexusAnimMontagePlayerComponent* AGunnerEquipment::GetAnimMontagePlayer_Implementation()
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

void AGunnerEquipment::AuthAddDesiredActions(const TArray<TSubclassOf<UNexusAction>>& ActionsToAdd, TArray<FNexusActionDefHandle>& AddedActionHandles)
{
	AActor* ActorOwner = GetOwner();
	check(ActorOwner);
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(ActorOwner);
	check(ActionComponent);

	for (auto ActionClass : ActionsToAdd)
	{
		if (ActionClass)
		{
			FNexusActionDef ActionDef(this, ActionClass);
			if (ActorOwner->HasAuthority())
			{
				AddedActionHandles.Add(ActionComponent->AuthAddAction(ActionDef));
			}
		}
	}
}

void AGunnerEquipment::AuthRemoveDesiredActions(TArray<FNexusActionDefHandle>& AddedActionHandles)
{
	AActor* ActorOwner = GetOwner();
	check(ActorOwner);
	if (!ActorOwner->HasAuthority())
	{
		return;
	}

	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(ActorOwner);
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

void AGunnerEquipment::OnTeamSetEvent(FGenericTeamId OldTeamID, FGenericTeamId NewTeamID)
{
	APawn* PawnOwner = GetOwner<APawn>();
	if (!PawnOwner->IsLocallyControlled())
	{
		for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PlayerController = Iterator->Get();
			if (PlayerController && PlayerController->IsLocalPlayerController())
			{
				IGunnerTeamAgentInterface* TeamAgentInterface = PlayerController->GetPlayerState<IGunnerTeamAgentInterface>();
				ETeamAttitude::Type Attitude = TeamAgentInterface->GetTeamAttitudeTowards(*PawnOwner);
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
		}
	}
}

void AGunnerEquipment::SetEquipmentDef(UGunnerEquipmentDef* InEquipmentDef)
{
	EquipmentDef = InEquipmentDef;
	BulletCount = EquipmentDef->Magazine;
	MaxBulletPerMagazineCount = EquipmentDef->Magazine;
	MagazineBulletCount = EquipmentDef->Reserve;
	Tags.Empty();
	Tags.Add(EquipmentDef->EquipmentName);
}
