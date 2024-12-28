// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentTraceHitComponent.h"

#include "GunnerEquipment.h"
#include "TraceHitMessageData.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "Gunner/_Core/LagCompensationComponent.h"
#include "Gunner/_Core/Event/GunnerEventManagerComponent.h"
#include "Gunner/_Core/Input/GunnerEventMessage.h"
#include "Kismet/GameplayStatics.h"


UEquipmentTraceHitComponent::UEquipmentTraceHitComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UEquipmentTraceHitComponent::BP_TraceHit(TArray<FHitResult>& OutHitResults)
{
	FCollisionQueryParams CollisionQueryParams;
	TArray<AActor*> IgnoredActors = {EquipmentOwner->GetOwner(), EquipmentOwner};
	CollisionQueryParams.AddIgnoredActors(IgnoredActors);
	TraceHit(OutHitResults, CollisionQueryParams);
}

void UEquipmentTraceHitComponent::BeginPlay()
{
	Super::BeginPlay();
	EquipmentOwner = Cast<AGunnerEquipment>(GetOwner());
	check(EquipmentOwner);
}

void UEquipmentTraceHitComponent::TraceHit(TArray<FHitResult>& OutHitResults, const FCollisionQueryParams& CollisionQueryParams)
{
	AActor* EquippedActor = EquipmentOwner->GetOwner();
	UWorld* World = EquippedActor->GetWorld();
	UCameraComponent* CameraComponet = EquippedActor->GetComponentByClass<UCameraComponent>();
	FVector CameraLocation = CameraComponet->GetComponentLocation();
	FVector CameraForward = CameraComponet->GetForwardVector();

	World->LineTraceMultiByChannel(OutHitResults,
	                               CameraLocation,
	                               CameraLocation + CameraForward * 10000.0f,
	                               ECollisionChannel::ECC_Visibility, CollisionQueryParams, FCollisionResponseParams(ECR_Overlap));

	if (!EquippedActor->HasAuthority())
	{
		TArray<AActor*> HitActors;
		for (const FHitResult& HitResult : OutHitResults)
		{
			if (AActor* HitActor = HitResult.GetActor())
			{
				HitActors.AddUnique(HitActor);
			}
		}
		ServerRequestHitScanConfirm(HitActors, World->GetTimeSeconds());
	}
	else
	{
		AuthApplyDamageByHitResults(OutHitResults);
	}
}

float UEquipmentTraceHitComponent::CalculateDamage_Implementation(const FHitResult& HitResult) const
{
	return 1.0f;
}

void UEquipmentTraceHitComponent::ServerRequestHitScanConfirm_Implementation(const TArray<AActor*>& HitActors, float TimeStamp)
{
	TArray<ACharacter*> LagCompensationTargetCharacters;
	for (AActor* HitActor : HitActors)
	{
		ACharacter* Character = Cast<ACharacter>(HitActor);
		if (Character && Character->GetComponentByClass<ULagCompensationComponent>())
		{
			LagCompensationTargetCharacters.AddUnique(Character);
		}
	}

	for (ACharacter* TargetCharacter : LagCompensationTargetCharacters)
	{
		ULagCompensationComponent* LagCompensationComponent = TargetCharacter->GetComponentByClass<ULagCompensationComponent>();
		check(LagCompensationComponent);
		LagCompensationComponent->AuthBeginRewind(TimeStamp);
	}


	FCollisionQueryParams CollisionQueryParams;
	TArray<AActor*> LagCompensatableActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), LagCompensatableActors);
	for (AActor* CompensatableActor : LagCompensatableActors)
	{
		ACharacter* Character = Cast<ACharacter>(CompensatableActor);
		check(Character);
		CollisionQueryParams.AddIgnoredComponent(Character->GetMesh());
	}

	TArray<FHitResult> HitResults;
	TArray<AActor*> IgnoredActors = {GetOwner(), EquipmentOwner};
	CollisionQueryParams.AddIgnoredActors(IgnoredActors);
	TraceHit(HitResults, CollisionQueryParams);


	for (ACharacter* TargetCharacter : LagCompensationTargetCharacters)
	{
		ULagCompensationComponent* LagCompensationComponent = TargetCharacter->GetComponentByClass<ULagCompensationComponent>();
		check(LagCompensationComponent);
		LagCompensationComponent->AuthEndRewind();
	}

	AuthApplyDamageByHitResults(HitResults);
}

void UEquipmentTraceHitComponent::AuthApplyDamageByHitResults(const TArray<FHitResult>& HitResults)
{
	TArray<AActor*> ActorsToApplyDamage;
	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		if (ActorsToApplyDamage.Find(HitActor) == INDEX_NONE)
		{
			ActorsToApplyDamage.Add(HitActor);
			AuthApplyDamage(HitActor, HitResult.BoneName, HitResult.ImpactNormal, CalculateDamage(HitResult));
		}
	}
}

void UEquipmentTraceHitComponent::AuthApplyDamage(AActor* HitActor, FName BoneName, FVector HitNormal, float DamageAmount)
{
	if (UGunnerEventManagerComponent* EventManagerComponent = UGunnerEventManagerComponent::GetEventManagerComponentFromActor(HitActor))
	{
		FGunnerEventMessage HitScanMessage;
		HitScanMessage.Instigator = GetOwner();
		UGunnerHitMessageData* HitMessageData = NewObject<UGunnerHitMessageData>();
		HitMessageData->HitBoneName = BoneName;
		HitMessageData->HitNormal = HitNormal;
		HitMessageData->HitEquipment = EquipmentOwner;
		HitMessageData->DamageAmount = DamageAmount;
		HitScanMessage.EventDataObject = HitMessageData;

		EventManagerComponent->SendEventToActor(FGameplayTag::RequestGameplayTag(FName("GameEvent.Damaged")), HitScanMessage, HitActor);
	}
}
