// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionDamaged.h"

#include "Gunner/Equipment/GunnerEquipmentManagerComponent.h"

UGunnerActionDamaged::UGunnerActionDamaged()
{
	HeadBoneNames = {TEXT("Head"), TEXT("Neck")};
	LegBoneNames = {TEXT("L_Hip"),TEXT("L_Knee"),TEXT("L_Foot"),TEXT("R_Hip"),TEXT("R_Knee"),TEXT("R_Foot")};
}

void UGunnerActionDamaged::OnTriggerAction_Implementation()
{
	Super::OnTriggerAction_Implementation();
	HitMessageData = Cast<UGunnerHitMessageData>(EventMessage.EventDataObject);
	check(HitMessageData.IsValid());
}

EGunnerHitDirectionType UGunnerActionDamaged::GetHitDirectionType() const
{
	check(EventMessage.Instigator);
	AActor* AgentActor = GetAgentActor();
	check(AgentActor);

	const FVector HitCauserLocation = EventMessage.Instigator->GetActorLocation();
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

EGunnerHitBoneType UGunnerActionDamaged::GetHitBoneType(FName HitBoneName) const
{
	if (HeadBoneNames.Contains(HitBoneName))
	{
		return EGunnerHitBoneType::Head;
	}

	if (LegBoneNames.Contains(HitBoneName))
	{
		return EGunnerHitBoneType::Leg;
	}

	return EGunnerHitBoneType::Body;
}

UAnimMontage* UGunnerActionDamaged::GetDesiredHitMontage(FName HitBoneName) const
{
	const EGunnerHitDirectionType HitDirectionType = GetHitDirectionType();
	const EGunnerHitBoneType HitBoneType = GetHitBoneType(HitBoneName);

	const FGunnerHitMontageSet* HitMontageSet = HitMontages.Find(HitBoneType);
	if (!HitMontageSet)
	{
		return nullptr;
	}


	const FGunnerHitMontage& HitMontage = HitMontageSet->MontageSet[MontageSetIndex];
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
