// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerAction_Death.h"

#include "Gunner/_Core/GunnerDirectionalMontage.h"
#include "Gunner/_Core/Damage/GunnerDamageContext.h"
#include "Gunner/_Core/GunnerHitBoxInterface.h"

void UGunnerAction_Death::OnTriggerAction()
{
	Super::OnTriggerAction();
	DamageContext = Cast<UGunnerDamageContext>(GetEventMessage().EventDataObject);
}

UAnimMontage* UGunnerAction_Death::GetDesiredDeathMontage(FName HitBoneName, bool bLarge) const
{
	EGunnerHitPartType HitBoxType = IGunnerHitBoxInterface::Execute_GetHitPartTypeByHitBoneName(GetAgentActor(), HitBoneName);

	const FGunnerDirectionalMontageSet* DeathMontageSet = DeathMontages.Find(HitBoxType);
	if (!DeathMontageSet)
	{
		return nullptr;
	}

	const FGunnerDirectionalMontage& DeathMontage = bLarge ? DeathMontageSet->MontageSet[1] : DeathMontageSet->MontageSet[0];

	EGunnerHitDirectionType HitDirectionType = EGunnerHitDirectionType::Front;
	if (DamageContext && DamageContext->Causer && DamageContext->Target)
	{
		HitDirectionType = IGunnerHitBoxInterface::GetHitDirectionType(
			DamageContext->Causer->GetActorLocation(),
			DamageContext->Target->GetActorLocation(),
			GetAgentActor()->GetActorForwardVector());
	}


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
