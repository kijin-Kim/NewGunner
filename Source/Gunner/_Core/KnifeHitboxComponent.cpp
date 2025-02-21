// Fill out your copyright notice in the Description page of Project Settings.


#include "KnifeHitboxComponent.h"

#include "GenericTeamAgentInterface.h"
#include "Camera/CameraComponent.h"
#include "Event/NexusEventManagerComponent.h"
#include "Gunner/Equipment/GunnerEquipment.h"
#include "Gunner/Equipment/TraceHitMessageData.h"
#include "Event/NexusEventMessage.h"


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
	if (UNexusEventManagerComponent* EventManagerComponent = UNexusEventManagerComponent::GetEventManagerComponentFromActor(HitActor))
	{
		FNexusEventMessage HitScanMessage;
		HitScanMessage.EventTag = FGameplayTag::RequestGameplayTag(FName("GameEvent.Damaged"));
		APawn* EquipmentPawnOwner = Cast<APawn>(GetOwner()->GetOwner());
		HitScanMessage.Instigator = EquipmentPawnOwner->GetController();
		UGunnerHitMessageData* HitMessageData = NewObject<UGunnerHitMessageData>();

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

		HitMessageData->HitBoneName = HitBoneName;
		HitMessageData->HitNormal = HitNormal;
		HitMessageData->HitEquipment = Cast<AGunnerEquipment>(GetOwner());
		HitMessageData->DamageAmount = 1.0f;
		HitScanMessage.EventDataObject = HitMessageData;

		EventManagerComponent->SendEventToActor(FGameplayTag::RequestGameplayTag(FName("GameEvent.Damaged")), HitScanMessage, HitActor);
	}
}
