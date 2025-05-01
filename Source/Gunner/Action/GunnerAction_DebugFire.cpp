// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerAction_DebugFire.h"

#include "GameFramework/Character.h"
#include "Gunner/_Core/GunnerActionComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"

TArray<FHitResult> UGunnerAction_DebugFire::HitScanTrace()
{
	const TArray<FHitResult> HitResults = Super::HitScanTrace();

	if (HitResults.IsEmpty())
	{
		return HitResults;
	}

	TArray<AActor*> HitActors = GetUniqueActorsFromHitResults(HitResults);
	TArray<AActor*> CharacterActors = HitActors.FilterByPredicate([](AActor* Actor)
	{
		return !Actor->IsA<ACharacter>();
	});

	TArray<ACharacter*> Characters;
	for (AActor* Actor : CharacterActors)
	{
		Characters.Add(Cast<ACharacter>(Actor));
	}

	if (!IsOwnerActorAuthoritative())
	{
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

				UGunnerAction_DebugFire::DrawDebugHitBoxData(GetWorld(), HitBoxData, FColor::Blue, true);
			}
		}
		return HitResults;
	}

	for (ACharacter* Character : Characters)
	{
		if (Character)
		{
			FGunnerDebugHitConfirmedDataEntry* ConfirmedPtr = DebugHitConfirmData.FindByPredicate([Character](const FGunnerDebugHitConfirmedDataEntry& Entry)
				{
					return Entry.ClientClaimedHitCharacter == Character;
				}
			);

			if (ConfirmedPtr)
			{
				ConfirmedPtr->CollectHitBoxData(Character->GetMesh()->GetPhysicsAsset()->SkeletalBodySetups);
				ConfirmedPtr->bHitConfirmed = true;
			}
		}
	}


	return HitResults;
}

void UGunnerAction_DebugFire::AuthHitScanTraceConfirm(const FNexusTargetDataHandle& HitTargetDataHandle)
{
	Super::AuthHitScanTraceConfirm(HitTargetDataHandle);
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetAgentActor());
	if (UGunnerActionComponent* GunnerActionComponent = Cast<UGunnerActionComponent>(ActionComponent))
	{
		GunnerActionComponent->ClientSendDebugHitConfirmedData(DebugHitConfirmData);
		DebugHitConfirmData.Empty();
	}
}

void UGunnerAction_DebugFire::DrawDebugHitBoxData(UWorld* World, const TArray<FGunnerDebugHitBoxDataEntry>& HitBoxData, const FColor& DebugDrawColor, bool bPersistentLines, float LifeTime)
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

void UGunnerAction_DebugFire::AuthOnBeginRewind(TArray<ACharacter*> LagCompensationTargetCharacters, float TimeStamp)
{
	for (ACharacter* Character : LagCompensationTargetCharacters)
	{
		FGunnerDebugHitConfirmedDataEntry& Entry = DebugHitConfirmData.AddDefaulted_GetRef();
		Entry.ClientClaimedHitCharacter = Character;
		Entry.CollectHitBoxData(Character->GetMesh()->GetPhysicsAsset()->SkeletalBodySetups);
	}

	Super::AuthOnBeginRewind(LagCompensationTargetCharacters, TimeStamp);
}
