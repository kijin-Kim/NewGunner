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

	if (!IsOwnerActorAuthoritative())
	{
		FlushPersistentDebugLines(GetWorld());
		FlushDebugStrings(GetWorld());
		for (ACharacter* Character : Characters)
		{
			if (Character && Character->GetMesh())
			{
				TArray<FGunnerDebugHitBoxDataEntry> HitBoxData;
				for (USkeletalBodySetup* BodySetup : Character->GetMesh()->GetPhysicsAsset()->SkeletalBodySetups)
				{
					FGunnerDebugHitBoxDataEntry& Entry = HitBoxData.AddDefaulted_GetRef();
					Entry.BoneName = BodySetup->BoneName;
					Entry.BoneTransform = Character->GetMesh()->GetBoneTransform(BodySetup->BoneName);
					Entry.AggGeom = BodySetup->AggGeom;
				}

				UGunnerAction_Fire::DrawDebugHitBoxData(GetWorld(), HitBoxData, FColor::Blue, true);
				FVector Location = Character->GetMesh()->GetComponentLocation();
				Location.Z += Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.0f + 10.0f;
				DrawDebugString(GetWorld(), Location, FString::Printf(TEXT("Client TimeStamp: %f"), GetWorld()->GetGameState()->GetServerWorldTimeSeconds()), nullptr, FColor::Blue, -1.0f, true);
			}
		}
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


	AuthOnBeginRewind(LagCompensationTargetCharacters, bEnableLagCompensation ? HitTargetData->TimeStamp : GetWorld()->GetTimeSeconds());


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


	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetAgentActor());
	if (UGunnerActionComponent* GunnerActionComponent = Cast<UGunnerActionComponent>(ActionComponent))
	{
		GunnerActionComponent->ClientSendDebugHitConfirmedData(DebugHitConfirmData);
		DebugHitConfirmData.Empty();
	}
}

void UGunnerAction_Fire::DrawDebugHitBoxData(UWorld* World, const TArray<FGunnerDebugHitBoxDataEntry>& HitBoxData, const FColor& DebugDrawColor, bool bPersistentLines, float LifeTime)
{
	for (const FGunnerDebugHitBoxDataEntry& Entry : HitBoxData)
	{
		for (const FKBoxElem& BoxElem : Entry.AggGeom.BoxElems)
		{
			FTransform ElemWorldTransform = BoxElem.GetTransform() * Entry.BoneTransform;
			FVector Extent = {BoxElem.X * 0.5f, BoxElem.Y * 0.5f, BoxElem.Z * 0.5f};
			DrawDebugBox(World, ElemWorldTransform.GetLocation(), Extent, DebugDrawColor, bPersistentLines, LifeTime);
		}

		for (const FKSphereElem& SphereElem : Entry.AggGeom.SphereElems)
		{
			FTransform ElemWorldTransform = SphereElem.GetTransform() * Entry.BoneTransform;
			DrawDebugSphere(World, ElemWorldTransform.GetLocation(), SphereElem.Radius, 12, DebugDrawColor, bPersistentLines, LifeTime);
		}

		for (const FKSphylElem& SphylElem : Entry.AggGeom.SphylElems)
		{
			FTransform ElemWorldTransform = SphylElem.GetTransform() * Entry.BoneTransform;
			DrawDebugCapsule(World, ElemWorldTransform.GetLocation(), SphylElem.GetScaledHalfLength(FVector(1.0f)), SphylElem.GetScaledRadius(FVector(1.0f)), ElemWorldTransform.GetRotation(), DebugDrawColor, bPersistentLines, LifeTime);
		}

		for (const FKTaperedCapsuleElem& TaperedCapsuleElem : Entry.AggGeom.TaperedCapsuleElems)
		{
			FTransform ElemWorldTransform = TaperedCapsuleElem.GetTransform() * Entry.BoneTransform;
			float Radius0;
			float Radius1;
			TaperedCapsuleElem.GetScaledRadii(FVector(1.0f), Radius0, Radius1);
			DrawDebugCapsule(World, ElemWorldTransform.GetLocation(), TaperedCapsuleElem.GetScaledHalfLength(FVector(1.0f)), Radius0, ElemWorldTransform.GetRotation(), DebugDrawColor, bPersistentLines, LifeTime);
		}
	}
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

void UGunnerAction_Fire::AuthOnBeginRewind(TArray<ACharacter*> LagCompensationTargetCharacters, float TimeStamp)
{
	for (ACharacter* TargetCharacter : LagCompensationTargetCharacters)
	{
		UGunnerLagCompensationComponent* LagCompensationComponent = TargetCharacter->GetComponentByClass<UGunnerLagCompensationComponent>();
		check(LagCompensationComponent);
		double RewoundTimeStamp;
		const bool bFoundSnapshot = LagCompensationComponent->AuthBeginRewind(TimeStamp, RewoundTimeStamp);
		if (bEnableDebug)
		{
			FGunnerDebugHitConfirmedDataEntry& Entry = DebugHitConfirmData.AddDefaulted_GetRef();
			Entry.ClientClaimedHitCharacter = TargetCharacter;
			Entry.bFoundSnapshot = bFoundSnapshot;
			Entry.CollectHitBoxData(TargetCharacter->GetMesh()->GetPhysicsAsset()->SkeletalBodySetups);
			Entry.ServerRewoundedTimeStamp = RewoundTimeStamp;
			Entry.ServerLocation = TargetCharacter->GetMesh()->GetComponentLocation();
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
				FGunnerDebugHitConfirmedDataEntry* ConfirmedPtr = DebugHitConfirmData.FindByPredicate([HitCharacter](const FGunnerDebugHitConfirmedDataEntry& Entry)
					{
						return Entry.ClientClaimedHitCharacter == HitCharacter;
					}
				);

				if (ConfirmedPtr)
				{
					ConfirmedPtr->CollectHitBoxData(SkeletalMeshComponent->GetPhysicsAsset()->SkeletalBodySetups);
					ConfirmedPtr->bHitConfirmed = true;
					ConfirmedPtr->ServerLocation = SkeletalMeshComponent->GetComponentLocation();
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
