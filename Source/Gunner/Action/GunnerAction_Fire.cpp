// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerAction_Fire.h"

#include "GunnerAction_Fire.h"

#include "GenericTeamAgentInterface.h"
#include "Action/NexusActionComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "Gunner/Item/GunnerGun.h"
#include "Gunner/_Core/GunnerLagCompensationComponent.h"
#include "TargetData/GunnerTargetData_Hit.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerState.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"
#include "Gunner/_Core/Damage/GunnerDamageContext.h"
#include "Gunner/_Core/Damage/GunnerDamageType.h"

TArray<FHitResult> UGunnerAction_Fire::HitScanTrace()
{
	APawn* AgentPawn = Cast<APawn>(GetAgentActor());
	UWorld* World = AgentPawn->GetWorld();
	UCameraComponent* CameraComponet = AgentPawn->GetComponentByClass<UCameraComponent>();
	FVector CameraLocation = CameraComponet->GetComponentLocation();
	FVector CameraForward = CameraComponet->GetForwardVector();

	FCollisionQueryParams CollisionQueryParams;
	TArray<AActor*> IgnoredActors = {GetSourceObject<AActor>(), AgentPawn};

	CollisionQueryParams.AddIgnoredActors(IgnoredActors);
	CollisionQueryParams.AddIgnoredActors(GetIgnoredActorsByTeam(AgentPawn->GetPlayerState()));

	TArray<FHitResult> HitResults;
	World->LineTraceMultiByChannel(HitResults,
	                               CameraLocation,
	                               CameraLocation + CameraForward * 10000.0f, // TODO: 설정파일을 통해 설정할 수 있도록 변경
	                               ECollisionChannel::ECC_Visibility, CollisionQueryParams, FCollisionResponseParams(ECR_Overlap));

	return HitResults;
}

void UGunnerAction_Fire::AuthHitScanTraceConfirm(const FNexusTargetDataHandle& HitTargetDataHandle)
{
	TSharedPtr<FGunnerTargetData_Hit> HitTargetData = StaticCastSharedPtr<FGunnerTargetData_Hit>(HitTargetDataHandle.GetData());
	check(HitTargetData.IsValid());

	TArray<AActor*> HitActors = GetUniqueActorsFromHitResults(HitTargetData->HitResults);

	TArray<ACharacter*> LagCompensationTargetCharacters;
	for (AActor* HitActor : HitActors)
	{
		ACharacter* Character = Cast<ACharacter>(HitActor);
		if (Character && Character->GetComponentByClass<UGunnerLagCompensationComponent>())
		{
			LagCompensationTargetCharacters.AddUnique(Character);
		}
	}

	AuthBeginRewind(LagCompensationTargetCharacters, HitTargetData->TimeStamp);


	// line trace portion
	FCollisionQueryParams CollisionQueryParams;
	TArray<AActor*> LagCompensatableActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), LagCompensatableActors);
	for (AActor* CompensatableActor : LagCompensatableActors)
	{
		ACharacter* Character = Cast<ACharacter>(CompensatableActor);
		check(Character);
		CollisionQueryParams.AddIgnoredComponent(Character->GetMesh());
	}
	
	APawn* AgentPawn = Cast<APawn>(GetAgentActor());
	TArray<AActor*> IgnoredActors = {GetSourceObject<AActor>(), AgentPawn};
	CollisionQueryParams.AddIgnoredActors(IgnoredActors);
	CollisionQueryParams.AddIgnoredActors(GetIgnoredActorsByTeam(AgentPawn->GetPlayerState()));

	TArray<FHitResult> HitResults = HitScanTrace();

	AuthEndRewind(LagCompensationTargetCharacters);

	AuthApplyDamageByHitResults(HitResults);
}

TArray<FHitResult> UGunnerAction_Fire::FilterDuplicateHitResultsByActor(const TArray<FHitResult>& HitResults)
{
	TArray<AActor*> HitActors;
	TArray<FHitResult> FilteredHitResults;
	for (const FHitResult& HitResult : HitResults)
	{
		if (!HitActors.Contains(HitResult.GetActor()))
		{
			HitActors.Add(HitResult.GetActor());
			FilteredHitResults.Add(HitResult);
		}
	}
	return FilteredHitResults;
}

TArray<AActor*> UGunnerAction_Fire::GetUniqueActorsFromHitResults(const TArray<FHitResult>& HitResults)
{
	FilterDuplicateHitResultsByActor(HitResults);
	TArray<AActor*> HitActors;
	for (const FHitResult& HitResult : HitResults)
	{
		HitActors.Add(HitResult.GetActor());
	}
	return HitActors;
}

TArray<AActor*> UGunnerAction_Fire::GetIgnoredActorsByTeam(APlayerState* PlayerState)
{
	TArray<AActor*> IgnoredActors;
	IGenericTeamAgentInterface* TeamAgentInterface = Cast<IGenericTeamAgentInterface>(PlayerState);
	if (!ensure(TeamAgentInterface))
	{
		return {};
	}

	TArray<AActor*> OtherAgents;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UGenericTeamAgentInterface::StaticClass(), OtherAgents);
	for (AActor* OtherAgent : OtherAgents)
	{
		if (TeamAgentInterface->GetTeamAttitudeTowards(*OtherAgent) != ETeamAttitude::Hostile)
		{
			IgnoredActors.Add(OtherAgent);
		}
	}

	return IgnoredActors;
}

void UGunnerAction_Fire::AuthBeginRewind(TArray<ACharacter*> LagCompensationTargetCharacters, float TimeStamp)
{
	for (ACharacter* TargetCharacter : LagCompensationTargetCharacters)
	{
		UGunnerLagCompensationComponent* LagCompensationComponent = TargetCharacter->GetComponentByClass<UGunnerLagCompensationComponent>();
		check(LagCompensationComponent);
		LagCompensationComponent->AuthBeginRewind(TimeStamp);
	}
}

void UGunnerAction_Fire::AuthEndRewind(TArray<ACharacter*> LagCompensationTargetCharacters)
{
	for (ACharacter* TargetCharacter : LagCompensationTargetCharacters)
	{
		UGunnerLagCompensationComponent* LagCompensationComponent = TargetCharacter->GetComponentByClass<UGunnerLagCompensationComponent>();
		check(LagCompensationComponent);
		LagCompensationComponent->AuthEndRewind();
	}
}

void UGunnerAction_Fire::AuthApplyDamageByHitResults(const TArray<FHitResult>& HitResults)
{
	for (const FHitResult& HitResult : FilterDuplicateHitResultsByActor(HitResults))
	{
		AuthApplyDamage(HitResult.GetActor(), HitResult.BoneName, HitResult.ImpactNormal);
	}
}

void UGunnerAction_Fire::AuthApplyDamage(AActor* HitActor, FName HitBoneName, FVector HitNormal)
{
	if (!DamageType)
	{
		return;
	}


	FNexusEventMessage DamageEventMessage;
	DamageEventMessage.EventTag = GunnerNativeGameplayTags::TAG_GameEvent_Damaged;
	APawn* AgentPawn = Cast<APawn>(GetAgentActor());
	DamageEventMessage.Instigator = AgentPawn->GetController();


	UGunnerDamageContext* DamageContext = NewObject<UGunnerDamageContext>();

	DamageContext->Instigator = AgentPawn->GetController();
	DamageContext->Causer = GetSourceObject<AActor>();
	DamageContext->Target = HitActor;
	DamageContext->HitNormal = HitNormal;
	DamageContext->HitBoneName = HitBoneName;

	DamageContext->DamageAmount = DamageType->CalculateDamageByContext(DamageContext);
	DamageEventMessage.EventDataObject = DamageContext;

	UNexusActionComponent::SendEventToActor<FNexusEventMessage>(GunnerNativeGameplayTags::TAG_GameEvent_Damaged, DamageEventMessage, HitActor);
}
