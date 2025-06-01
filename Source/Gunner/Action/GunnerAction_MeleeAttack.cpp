// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerAction_MeleeAttack.h"

#include "GenericTeamAgentInterface.h"
#include "Action/NexusActionComponent.h"
#include "Animation/NexusAnimMontagePlayerComponent.h"
#include "Animation/NexusAnimMontagePlayerInterface.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"
#include "Gunner/_Core/Damage/GunnerDamageContext.h"
#include "Gunner/_Core/Damage/GunnerDamageType.h"

void UGunnerAction_MeleeAttack::OnAddAction()
{
	Super::OnAddAction();
	if (IsOwnerActorAuthoritative())
	{
		AActor* MeleeItemActor = GetSourceObject<AActor>();
		SphereComponent = Cast<USphereComponent>(MeleeItemActor->AddComponentByClass(USphereComponent::StaticClass(), true, FTransform::Identity, false));
		SphereComponent->AttachToComponent(INexusAnimMontagePlayerInterface::Execute_GetFirstPersonMeshComponent(MeleeItemActor), FAttachmentTransformRules::KeepRelativeTransform);
		SphereComponent->SetSphereRadius(SphereRadius);
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &UGunnerAction_MeleeAttack::OnBeginOverlap);
	}
}

void UGunnerAction_MeleeAttack::OnEndAction()
{
	Super::OnEndAction();
	if (IsOwnerActorAuthoritative())
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void UGunnerAction_MeleeAttack::OnRemoveAction()
{
	Super::OnRemoveAction();
	if (IsOwnerActorAuthoritative())
	{
		if (SphereComponent)
		{
			SphereComponent->DestroyComponent();
			SphereComponent = nullptr;
		}
	}
}

void UGunnerAction_MeleeAttack::StartHitDetection()
{
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->UpdateOverlaps();
}

void UGunnerAction_MeleeAttack::StopHitDetection()
{
	AlreadyHitDetectedActors.Empty();
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UGunnerAction_MeleeAttack::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AActor* AgentActor = GetAgentActor();
	check(AgentActor);

	if (IGenericTeamAgentInterface* EquippedOwnerTeamAgentInterface = Cast<IGenericTeamAgentInterface>(AgentActor))
	{
		if (EquippedOwnerTeamAgentInterface->GetTeamAttitudeTowards(*OtherActor) != ETeamAttitude::Hostile)
		{
			return;
		}
	}

	if (GetOwnerActor() != OtherActor || AgentActor != OtherActor || !AlreadyHitDetectedActors.Contains(OtherActor))
	{
		AlreadyHitDetectedActors.Add(OtherActor);
		AuthApplyDamage(OtherActor);
	}
}


void UGunnerAction_MeleeAttack::AuthApplyDamage(AActor* HitActor)
{
	if (DamageType)
	{
		AActor* AgentActor = GetAgentActor();
		UWorld* World = AgentActor->GetWorld();

		UCameraComponent* CameraComponet = AgentActor->GetComponentByClass<UCameraComponent>();
		FVector CameraLocation;
		FRotator CameraRotation;
		AgentActor->GetActorEyesViewPoint(CameraLocation, CameraRotation);
		FVector CameraForward = CameraRotation.Vector();

		FCollisionQueryParams CollisionQueryParams;
		TArray<AActor*> IgnoredActors = {AgentActor, GetOwnerActor()};
		CollisionQueryParams.AddIgnoredActors(IgnoredActors);
		TArray<FHitResult> OutHitResults;
		World->LineTraceMultiByChannel(OutHitResults,
		                               CameraLocation,
		                               CameraLocation + CameraForward * 1000.0f,
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


		FNexusEventMessage DamageEventMessage;
		DamageEventMessage.EventTag = GunnerNativeGameplayTags::TAG_GameEvent_ApplyDamage;
		APawn* AgentPawn = Cast<APawn>(GetAgentActor());
		DamageEventMessage.Instigator = GetAgentActor();
		DamageEventMessage.TargetActor = HitActor;
		DamageEventMessage.Location = HitResultPtr->Location;
		DamageEventMessage.Normal = HitNormal;
		DamageEventMessage.HitResults = OutHitResults;

		FGunnerDamageContext DamageContext;
		DamageContext.Instigator = GetAgentActor();
		DamageContext.Target = HitActor;
		DamageContext.HitNormal = HitNormal;
		DamageContext.HitBoneName = HitBoneName;
		DamageEventMessage.Amount = DamageType->CalculateDamageByContext(DamageContext);
		
		
		UNexusActionComponent::SendEventToActor<FNexusEventMessage>(GunnerNativeGameplayTags::TAG_GameEvent_ApplyDamage, DamageEventMessage, HitActor);
	}
}
