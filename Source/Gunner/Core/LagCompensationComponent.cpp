// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"

#include "Containers/RingBuffer.h"
#include "Gunner/Gunner.h"
#include "Gunner/GunnerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicsAsset.h"


ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	const double MaxRewindTime = 0.3;
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		TArray<AActor*> GunnerActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGunnerCharacter::StaticClass(), GunnerActors);
		

		if (GunnerActors.IsEmpty())
		{
			return;
		}
		
		
		while(!HitBoxHistories.IsEmpty() && GetWorld()->GetTimeSeconds() - HitBoxHistories.Last().Time >= MaxRewindTime)
		{
			HitBoxHistories.Pop();
		}
		

		FHitBoxHistory NewHistory;
		NewHistory.Time = GetWorld()->GetTimeSeconds();

		for (AActor* GunnerActor : GunnerActors)
		{
			AGunnerCharacter* HitGunner = Cast<AGunnerCharacter>(GunnerActor);
			check(HitGunner);
			UPhysicsAsset* PhysAsset = HitGunner->GetMesh()->GetPhysicsAsset();
			check(PhysAsset);


			TArray<FHitBox>& HitBoxes = NewHistory.HitBoxes.FindOrAdd(GunnerActor);
			for (const USkeletalBodySetup* BodySetup : PhysAsset->SkeletalBodySetups)
			{
				FTransform BodyTransform = HitGunner->GetMesh()->GetSocketTransform(BodySetup->BoneName);
				for (const FKSphylElem& SphylElem : BodySetup->AggGeom.SphylElems)
				{
					FTransform HitBoxTransform = SphylElem.GetTransform() * BodyTransform;
					HitBoxes.Add({
						.Transform = HitBoxTransform,
						.HalfHeight = SphylElem.GetScaledHalfLength(FVector(1.0f, 1.0f, 1.0f)),
						.Radius = SphylElem.GetScaledRadius(FVector(1.0f, 1.0f, 1.0f)),
						.BoneName = BodySetup->BoneName,
					});
				}
			}

			for (const auto& [Transform, HalfHeight, Radius, BoneName] : HitBoxes)
			{
				DrawDebugCapsule(GetWorld(), Transform.GetLocation(), HalfHeight, Radius, Transform.GetRotation(), FColor::Green, false, MaxRewindTime);
			}
		}

		HitBoxHistories.AddFront(NewHistory);
	}
}
