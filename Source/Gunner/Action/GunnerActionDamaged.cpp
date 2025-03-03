// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionDamaged.h"
#include "Gunner/_Core/Damage/GunnerDamageContext.h"
#include "Gunner/_Core/GunnerHitBoxInterface.h"

void UGunnerActionDamaged::OnTriggerAction()
{
	Super::OnTriggerAction();
	DamageContext = Cast<UGunnerDamageContext>(GetEventMessage().EventDataObject);

	check(DamageContext);
}

EGunnerHitDirectionType UGunnerActionDamaged::GetHitDirectionType() const
{
	check(GetEventMessage().Instigator);
	AActor* AgentActor = GetAgentActor();
	check(AgentActor);

	APawn* HitCauser = GetEventMessage().Instigator->GetPawn();
	check(HitCauser);
	const FVector HitCauserLocation = HitCauser->GetActorLocation();
	const FVector AgentLocation = AgentActor->GetActorLocation();
	const FVector ToHitCauser = (HitCauserLocation - AgentLocation).GetSafeNormal2D();
	const FVector AgentForward = AgentActor->GetActorForwardVector();

	const float DotFront = ToHitCauser.Dot(AgentForward);
	const float CrossZ = AgentForward.Cross(ToHitCauser).Z;

	if (abs(DotFront) > abs(CrossZ))
	{
		return DotFront > 0.0f ? EGunnerHitDirectionType::Front : EGunnerHitDirectionType::Back;
	}

	return CrossZ > 0.0f ? EGunnerHitDirectionType::Right : EGunnerHitDirectionType::Left;
}


UAnimMontage* UGunnerActionDamaged::GetDesiredHitMontage(FName HitBoneName) const
{
	const EGunnerHitDirectionType HitDirectionType = IGunnerHitBoxInterface::GetHitDirectionType(
		DamageContext->Causer->GetActorLocation(),
		DamageContext->Target->GetActorLocation(),
		GetAgentActor()->GetActorForwardVector());
	EGunnerHitBoxType HitBoxType = IGunnerHitBoxInterface::Execute_GetHitBoxTypeByHitBoneName(GetAgentActor(), HitBoneName);
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

UAnimMontage* UGunnerActionDamaged::GetDesiredDeathMontage(FName HitBoneName, bool bLarge) const
{
	EGunnerHitBoxType HitBoxType = IGunnerHitBoxInterface::Execute_GetHitBoxTypeByHitBoneName(GetAgentActor(), HitBoneName);

	const FGunnerDirectionalMontageSet* DeathMontageSet = DeathMontages.Find(HitBoxType);
	if (!DeathMontageSet)
	{
		return nullptr;
	}

	const FGunnerDirectionalMontage& DeathMontage = bLarge ? DeathMontageSet->MontageSet[1] : DeathMontageSet->MontageSet[0];

	switch (GetHitDirectionType())
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
