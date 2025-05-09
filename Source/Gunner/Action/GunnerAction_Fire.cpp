// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerAction_Fire.h"

#include "GunnerAction_Fire.h"

#include "GenericTeamAgentInterface.h"
#include "Action/NexusActionComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Gunner/_Core/GunnerActionComponent.h"
#include "Gunner/_Core/GunnerLagCompensationComponent.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"
#include "Gunner/_Core/Damage/GunnerDamageContext.h"
#include "Gunner/_Core/Damage/GunnerDamageType.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "TargetData/GunnerTargetData_Hit.h"


TArray<FHitResult> UGunnerAction_Fire::HitScanTrace()
{
	FCollisionQueryParams CollisionQueryParams;

	APawn* AgentPawn = Cast<APawn>(GetAgentActor());
	TArray<AActor*> IgnoredActors = {GetSourceObject<AActor>(), AgentPawn};

	CollisionQueryParams.AddIgnoredActors(IgnoredActors);
	CollisionQueryParams.AddIgnoredActors(GetIgnoredActorsByTeam(AgentPawn->GetPlayerState()));

	TArray<FHitResult> HitResults;
	FVector TraceStart;
	FVector TraceEnd;
	CalculateTraceStartEnd(TraceStart, TraceEnd);
	GetWorld()->LineTraceMultiByChannel(HitResults, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, CollisionQueryParams, FCollisionResponseParams(ECR_Overlap));


	if (bEnableDebug && !IsOwnerActorAuthoritative())
	{
		DrawDebugHitScanTrace(HitResults);
	}


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

	float RoundTripTime = 0.0f;
	AController* Controller = GetController();
	if (Controller && Controller->PlayerState)
	{
		RoundTripTime = Controller->PlayerState->GetPingInMilliseconds() * 0.001f;
	}

	const float TimeStamp = GetWorld()->GetTimeSeconds() - RoundTripTime;
	AuthOnBeginRewind(LagCompensationTargetCharacters, bEnableLagCompensation ? TimeStamp : GetWorld()->GetTimeSeconds());


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


	AuthOnEndRewind(LagCompensationTargetCharacters, HitResults);

	AuthApplyDamageByHitResults(HitResults);


	if (bEnableDebug)
	{
		UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetAgentActor());
		if (UGunnerActionComponent* GunnerActionComponent = Cast<UGunnerActionComponent>(ActionComponent))
		{
			GunnerActionComponent->ClientSendDebugHitConfirmedData(DebugHitConfirmInfos);
			DebugHitConfirmInfos.Empty();
		}
	}
}

void UGunnerAction_Fire::DrawDebugHitBoxData(UWorld* World, const TArray<FGunnerDebugHitBoxInfo>& HitBoxData, const FColor& DebugDrawColor, bool bPersistentLines, float LifeTime)
{
	for (const FGunnerDebugHitBoxInfo& Entry : HitBoxData)
	{
		for (const FKBoxElem& BoxElem : Entry.AggGeom.BoxElems)
		{
			FTransform ElemWorldTransform = BoxElem.GetTransform() * Entry.BoneWorldTransform;
			FVector Extent = {BoxElem.X * 0.5f, BoxElem.Y * 0.5f, BoxElem.Z * 0.5f};
			DrawDebugBox(World, ElemWorldTransform.GetLocation(), Extent, DebugDrawColor, bPersistentLines, LifeTime);
		}

		for (const FKSphereElem& SphereElem : Entry.AggGeom.SphereElems)
		{
			FTransform ElemWorldTransform = SphereElem.GetTransform() * Entry.BoneWorldTransform;
			DrawDebugSphere(World, ElemWorldTransform.GetLocation(), SphereElem.Radius, 12, DebugDrawColor, bPersistentLines, LifeTime);
		}

		for (const FKSphylElem& SphylElem : Entry.AggGeom.SphylElems)
		{
			FTransform ElemWorldTransform = SphylElem.GetTransform() * Entry.BoneWorldTransform;
			DrawDebugCapsule(World, ElemWorldTransform.GetLocation(), SphylElem.GetScaledHalfLength(FVector(1.0f)), SphylElem.GetScaledRadius(FVector(1.0f)), ElemWorldTransform.GetRotation(), DebugDrawColor, bPersistentLines, LifeTime);
		}

		for (const FKTaperedCapsuleElem& TaperedCapsuleElem : Entry.AggGeom.TaperedCapsuleElems)
		{
			FTransform ElemWorldTransform = TaperedCapsuleElem.GetTransform() * Entry.BoneWorldTransform;
			float Radius0;
			float Radius1;
			TaperedCapsuleElem.GetScaledRadii(FVector(1.0f), Radius0, Radius1);
			DrawDebugCapsule(World, ElemWorldTransform.GetLocation(), TaperedCapsuleElem.GetScaledHalfLength(FVector(1.0f)), Radius0, ElemWorldTransform.GetRotation(), DebugDrawColor, bPersistentLines, LifeTime);
		}
	}
}


TArray<AActor*> UGunnerAction_Fire::GetUniqueActorsFromHitResults(const TArray<FHitResult>& HitResults)
{
	TSet<AActor*> UniqueActors;
	for (const FHitResult& HitResult : HitResults)
	{
		if (AActor* HitActor = HitResult.GetActor())
		{
			UniqueActors.Add(HitActor);
		}
	}
	return UniqueActors.Array();
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

void UGunnerAction_Fire::AuthOnBeginRewind(TArray<ACharacter*> LagCompensationTargetCharacters, float TargetTimeStamp)
{
	for (ACharacter* TargetCharacter : LagCompensationTargetCharacters)
	{
		UGunnerLagCompensationComponent* LagCompensationComponent = TargetCharacter->GetComponentByClass<UGunnerLagCompensationComponent>();
		check(LagCompensationComponent);
		if (!bEnableDebug)
		{
			LagCompensationComponent->AuthBeginRewind(TargetTimeStamp);
		}
		else
		{
			FGunnerDebugHitConfirmInfo& HitConfirmInfo = DebugHitConfirmInfos.AddDefaulted_GetRef();
			HitConfirmInfo.TargetCharacter = TargetCharacter;
			LagCompensationComponent->AuthBeginRewind(TargetTimeStamp, &HitConfirmInfo);
		}
	}
}

void UGunnerAction_Fire::AuthOnEndRewind(TArray<ACharacter*> LagCompensationTargetCharacters, const TArray<FHitResult>& HitResults)
{
	if (bEnableDebug)
	{
		for (const FHitResult& HitResult : HitResults)
		{
			USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(HitResult.GetComponent());
			if (!SkeletalMeshComponent)
			{
				continue;
			}

			if (!HitResult.GetActor())
			{
				continue;
			}

			if (ACharacter* HitCharacter = Cast<ACharacter>(HitResult.GetActor()))
			{
				FGunnerDebugHitConfirmInfo* ConfirmedPtr = DebugHitConfirmInfos.FindByPredicate([HitCharacter](const FGunnerDebugHitConfirmInfo& Entry)
					{
						return Entry.TargetCharacter == HitCharacter;
					}
				);

				if (ConfirmedPtr)
				{
					ConfirmedPtr->bServerConfirmedHit = true;
				}
			}
		}
	}


	for (ACharacter* TargetCharacter : LagCompensationTargetCharacters)
	{
		UGunnerLagCompensationComponent* LagCompensationComponent = TargetCharacter->GetComponentByClass<UGunnerLagCompensationComponent>();
		check(LagCompensationComponent);
		LagCompensationComponent->AuthEndRewind();
	}
}

void UGunnerAction_Fire::AuthApplyDamageByHitResults(const TArray<FHitResult>& HitResults)
{
	TSet<AActor*> DamagedActors;
	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor && !DamagedActors.Contains(HitActor))
		{
			AuthApplyDamage(HitActor, HitResult.BoneName, HitResult.ImpactNormal, HitResult);
			DamagedActors.Add(HitActor);
		}
	}
}

void UGunnerAction_Fire::AuthApplyDamage(AActor* HitActor, FName HitBoneName, FVector HitNormal, const FHitResult& HitResult)
{
	if (DamageType)
	{
		FNexusEventMessage DamageEventMessage;
		DamageEventMessage.EventTag = GunnerNativeGameplayTags::TAG_GameEvent_Damaged;
		APawn* AgentPawn = Cast<APawn>(GetAgentActor());
		DamageEventMessage.Instigator =  GetAgentActor();
		DamageEventMessage.TargetActor = HitActor;
		DamageEventMessage.Location = HitActor->GetActorLocation();
		DamageEventMessage.Normal = HitNormal;
		DamageEventMessage.HitResults.Add(HitResult);

		FGunnerDamageContext DamageContext;
		DamageContext.Instigator = GetAgentActor();
		DamageContext.Target = HitActor;
		DamageContext.HitNormal = HitNormal;
		DamageContext.HitBoneName = HitBoneName;
		DamageEventMessage.Amount = DamageType->CalculateDamageByContext(DamageContext);


		UNexusActionComponent::SendEventToActor<FNexusEventMessage>(GunnerNativeGameplayTags::TAG_GameEvent_Damaged, DamageEventMessage, HitActor);
	}
}


void UGunnerAction_Fire::DrawDebugHitScanTrace(const TArray<FHitResult>& HitResults)
{
	TArray<AActor*> HitActors = GetUniqueActorsFromHitResults(HitResults);
	TArray<AActor*> CharacterActors = HitActors.FilterByPredicate([](AActor* Actor)
	{
		return Actor->IsA<ACharacter>();
	});

	TArray<ACharacter*> Characters;
	for (AActor* Actor : CharacterActors)
	{
		Characters.Add(Cast<ACharacter>(Actor));
	}

	FlushPersistentDebugLines(GetWorld());
	FlushDebugStrings(GetWorld());
	for (ACharacter* Character : Characters)
	{
		if (Character && Character->GetMesh())
		{
			TArray<FGunnerDebugHitBoxInfo> HitBoxData;
			for (USkeletalBodySetup* BodySetup : Character->GetMesh()->GetPhysicsAsset()->SkeletalBodySetups)
			{
				FGunnerDebugHitBoxInfo& Entry = HitBoxData.AddDefaulted_GetRef();
				Entry.BoneName = BodySetup->BoneName;
				Entry.BoneWorldTransform = Character->GetMesh()->GetBoneTransform(BodySetup->BoneName);
				Entry.AggGeom = BodySetup->AggGeom;
			}

			UGunnerAction_Fire::DrawDebugHitBoxData(GetWorld(), HitBoxData, FColor::Blue, true);
			FVector StringLocation = Character->GetMesh()->GetComponentLocation();
			StringLocation.Z += Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.0f;

			float RoundTripTime = 0.0f;
			AController* Controller = GetController();
			if (Controller && Controller->PlayerState)
			{
				RoundTripTime = Controller->PlayerState->GetPingInMilliseconds() * 0.001f;
			}

			FString DebugString = FString::Printf(TEXT("TimeStamp: %.2f"), GetWorld()->GetGameState()->GetServerWorldTimeSeconds() + RoundTripTime * 0.5f);
			DrawDebugString(GetWorld(), StringLocation, DebugString, nullptr, FColor::Blue, -1.0f, true);
		}
	}
}

void UGunnerAction_Fire::CalculateTraceStartEnd(FVector& OutTraceStart, FVector& OutTraceEnd) const
{
	const APawn* AgentPawn = Cast<APawn>(GetAgentActor());
	if (!AgentPawn)
	{
		return;
	}

	const UCameraComponent* CameraComponent = AgentPawn->GetComponentByClass<UCameraComponent>();
	if (!CameraComponent)
	{
		return;
	}


	OutTraceStart = CameraComponent->GetComponentLocation();
	const FVector CameraForward = CameraComponent->GetForwardVector();
	const float MaxTraceDistance = 10000.0f;
	OutTraceEnd = OutTraceStart + CameraForward * MaxTraceDistance;
}
