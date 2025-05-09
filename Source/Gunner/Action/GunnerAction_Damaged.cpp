// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerAction_Damaged.h"
#include "Gunner/_Core/GunnerHitBoxInterface.h"


UAnimMontage* UGunnerAction_Damaged::GetDesiredHitMontage() const
{
	if (!EventMessage.Instigator || EventMessage.HitResults.IsEmpty())
	{
		return nullptr;
	}


	const FName HitBoneName = EventMessage.HitResults[0].BoneName;
	const FVector CauserLocation = EventMessage.Instigator->GetActorLocation();
	const FVector TargetLocation = GetAgentActor()->GetActorLocation();


	const EGunnerHitDirectionType HitDirectionType = IGunnerHitBoxInterface::GetHitDirectionType(
		CauserLocation,
		TargetLocation,
		GetAgentActor()->GetActorForwardVector());

	EGunnerHitPartType HitBoxType = IGunnerHitBoxInterface::Execute_GetHitPartTypeByHitBoneName(GetAgentActor(), HitBoneName);
	const FGunnerDirectionalMontageSet* HitMontageSet = HitMontages.Find(HitBoxType);
	if (!HitMontageSet)
	{
		return nullptr;
	}


	const FGunnerDirectionalMontage& HitMontage = HitMontageSet->MontageSet[MontageSetIndex];
	MontageSetIndex = (MontageSetIndex + 1) % HitMontageSet->MontageSet.Num();


	switch (HitDirectionType)
	{
	case EGunnerHitDirectionType::Front:
		return HitMontage.Front.Get();
	case EGunnerHitDirectionType::Back:
		return HitMontage.Back.Get();
	case EGunnerHitDirectionType::Left:
		return HitMontage.Left.Get();
	case EGunnerHitDirectionType::Right:
		return HitMontage.Right.Get();
	default:
		return nullptr;
	}
}
