// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionDeath.h"

#include "Gunner/_Core/GunnerDirectionalMontage.h"
#include "Gunner/_Core/Damage/GunnerDamageContext.h"
#include "Gunner/_Core/GunnerHitBoxInterface.h"

void UGunnerActionDeath::OnTriggerAction()
{
	Super::OnTriggerAction();
	DamageContext = Cast<UGunnerDamageContext>(GetEventMessage().EventDataObject);
	check(DamageContext);
}

UAnimMontage* UGunnerActionDeath::GetDesiredDeathMontage(FName HitBoneName, bool bLarge) const
{
	EGunnerHitBoxType HitBoxType = IGunnerHitBoxInterface::Execute_GetHitBoxTypeByHitBoneName(GetAgentActor(), HitBoneName);

	const FGunnerDirectionalMontageSet* DeathMontageSet = DeathMontages.Find(HitBoxType);
	if (!DeathMontageSet)
	{
		return nullptr;
	}

	const FGunnerDirectionalMontage& DeathMontage = bLarge ? DeathMontageSet->MontageSet[1] : DeathMontageSet->MontageSet[0];
	
	const EGunnerHitDirectionType HitDirectionType = IGunnerHitBoxInterface::GetHitDirectionType(
		DamageContext->Causer->GetActorLocation(),
		DamageContext->Target->GetActorLocation(),
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
