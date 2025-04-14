// Fill out your copyright notice in the Description page of Project Settings.


#include "KnifeHitboxComponent.h"

#include "GenericTeamAgentInterface.h"
#include "GunnerNativeGameplayTags.h"
#include "Action/NexusActionComponent.h"
#include "Camera/CameraComponent.h"
#include "Damage/GunnerDamageType.h"
#include "Event/NexusEventManagerComponent.h"
#include "Gunner/Equipment/GunnerEquipment.h"
#include "Gunner/_Core/Damage/GunnerDamageContext.h"
#include "Event/NexusEventMessage.h"
#include "Gunner/Equipment/GunnerEquipmentDef.h"


void UKnifeHitboxComponent::BeginPlay()
{
	Super::BeginPlay();
	OnComponentBeginOverlap.AddDynamic(this, &UKnifeHitboxComponent::OnBeginOverlap);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UKnifeHitboxComponent::StartHitDetection()
{
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ClearComponentOverlaps(true, false);
	UpdateOverlaps();
}

void UKnifeHitboxComponent::StopHitDetection()
{
	AlreadyHitDetectedActors.Empty();
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UKnifeHitboxComponent::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AActor* Owner = GetOwner();
	check(Owner);
	AActor* EquippedActor = Owner->GetOwner();
	check(EquippedActor);

	if (IGenericTeamAgentInterface* EquippedOwnerTeamAgentInterface = Cast<IGenericTeamAgentInterface>(EquippedActor))
	{
		if (EquippedOwnerTeamAgentInterface->GetTeamAttitudeTowards(*OtherActor) != ETeamAttitude::Hostile)
		{
			return;
		}
	}


	if (GetOwner() == OtherActor || EquippedActor == OtherActor || AlreadyHitDetectedActors.Contains(OtherActor))
	{
		return;
	}
	AlreadyHitDetectedActors.Add(OtherActor);

	if (EquippedActor->HasAuthority())
	{
		AuthApplyDamage(OtherActor);
	}
}

void UKnifeHitboxComponent::AuthApplyDamage(AActor* HitActor)
{
	
	FNexusEventMessage DamageEventMessage;
	DamageEventMessage.EventTag = TAG_GameEvent_Damaged;
	APawn* EquipmentPawnOwner = Cast<APawn>(GetOwner()->GetOwner());
	DamageEventMessage.Instigator = EquipmentPawnOwner->GetController();


	AActor* EquippedActor = GetOwner()->GetOwner();
	UWorld* World = EquippedActor->GetWorld();
	UCameraComponent* CameraComponet = EquippedActor->GetComponentByClass<UCameraComponent>();
	FVector CameraLocation = CameraComponet->GetComponentLocation();
	FVector CameraForward = CameraComponet->GetForwardVector();

	FCollisionQueryParams CollisionQueryParams;
	TArray<AActor*> IgnoredActors = {GetOwner()->GetOwner(), GetOwner()};
	CollisionQueryParams.AddIgnoredActors(IgnoredActors);
	TArray<FHitResult> OutHitResults;
	World->LineTraceMultiByChannel(OutHitResults,
	                               CameraLocation,
	                               CameraLocation + CameraForward * 10000.0f,
	                               ECollisionChannel::ECC_Visibility, CollisionQueryParams, FCollisionResponseParams(ECR_Overlap));
	FHitResult* HitResultPtr = OutHitResults.FindByPredicate([HitActor](const FHitResult& HitResult) { return HitResult.GetActor() == HitActor; });
	check(HitResultPtr);
	FName HitBoneName = TEXT("None");
	FVector HitNormal = FVector::ZeroVector;
	if (HitResultPtr)
	{
		HitBoneName = HitResultPtr->BoneName;
		HitNormal = HitResultPtr->ImpactNormal;
	}

	UGunnerDamageContext* DamageContext = NewObject<UGunnerDamageContext>();
	AGunnerEquipment* EquipmentOwner = GetOwner<AGunnerEquipment>();
	DamageContext->Instigator = EquipmentPawnOwner->GetController();
	DamageContext->Causer = EquipmentOwner;
	DamageContext->Target = HitActor;
	DamageContext->HitNormal = HitNormal;
	DamageContext->HitBoneName = HitBoneName;

	DamageContext->DamageAmount = EquipmentOwner->GetEquipmentDef()->CalculateDamageByContext(DamageContext);
	DamageEventMessage.EventDataObject = DamageContext;

	UNexusActionComponent::SendEventToActor(TAG_GameEvent_Damaged, DamageEventMessage, HitActor);
}
