// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerAction_Death.h"

#include "Gunner/_Core/GunnerDirectionalMontage.h"
#include "Gunner/_Core/GunnerHitBoxInterface.h"


UAnimMontage* UGunnerAction_Death::GetDesiredDeathMontage(bool bLarge) const
{
	
	if (EventMessage.HitResults.IsEmpty() || !EventMessage.Instigator)
	{
		return nullptr;
	}
	

	const FName HitBoneName = EventMessage.HitResults[0].BoneName;
	EGunnerHitPartType HitBoxType = IGunnerHitBoxInterface::Execute_GetHitPartTypeByHitBoneName(GetAgentActor(), HitBoneName);

	const FGunnerDirectionalMontageSet* DeathMontageSet = DeathMontages.Find(HitBoxType);
	if (!DeathMontageSet)
	{
		return nullptr;
	}

	const FGunnerDirectionalMontage& DeathMontage = bLarge ? DeathMontageSet->MontageSet[1] : DeathMontageSet->MontageSet[0];

	const FVector CauserLocation = EventMessage.Instigator->GetActorLocation();
	const FVector TargetLocation = GetAgentActor()->GetActorLocation();

	const EGunnerHitDirectionType HitDirectionType = IGunnerHitBoxInterface::GetHitDirectionType(
		CauserLocation,
		TargetLocation,
		GetAgentActor()->GetActorForwardVector());

	switch (HitDirectionType)
	{
	case EGunnerHitDirectionType::Front:
		return DeathMontage.Front.Get();
	case EGunnerHitDirectionType::Back:
		return DeathMontage.Back.Get();
	case EGunnerHitDirectionType::Left:
		return DeathMontage.Left.Get();
	case EGunnerHitDirectionType::Right:
		return DeathMontage.Right.Get();
	default:
		return nullptr;
	}
}
