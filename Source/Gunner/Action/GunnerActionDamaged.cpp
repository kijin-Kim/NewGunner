// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionDamaged.h"
#include "Gunner/Equipment/TraceHitMessageData.h"

UGunnerActionDamaged::UGunnerActionDamaged()
{
	HeadBoneNames = {TEXT("Head"), TEXT("Neck")};
	LegBoneNames = {TEXT("L_Hip"),TEXT("L_Knee"),TEXT("L_Foot"),TEXT("R_Hip"),TEXT("R_Knee"),TEXT("R_Foot")};
}

void UGunnerActionDamaged::OnTriggerAction()
{
	Super::OnTriggerAction();
	HitMessageData = Cast<UGunnerHitMessageData>(GetEventMessage().EventDataObject);
	check(HitMessageData);
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

FString UGunnerActionDamaged::GetHitBoneTypeAsString(FName HitBoneName) const
{
	EGunnerHitBoneType HitBoneType = GetHitBoneType(HitBoneName);
	switch (HitBoneType)
	{
	case EGunnerHitBoneType::Head:
		return TEXT("Head");
	case EGunnerHitBoneType::Body:
		return TEXT("Body");
	case EGunnerHitBoneType::Leg:
		return TEXT("Leg");
	default:
		return TEXT("None");
	}
	checkNoEntry();
}

UAnimMontage* UGunnerActionDamaged::GetDesiredHitMontage(FName HitBoneName) const
{
	const EGunnerHitDirectionType HitDirectionType = GetHitDirectionType();
	const EGunnerHitBoneType HitBoneType = GetHitBoneType(HitBoneName);

	const FGunnerDirectionalMontageSet* HitMontageSet = HitMontages.Find(HitBoneType);
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
	const EGunnerHitBoneType HitBoneType = GetHitBoneType(HitBoneName);

	const FGunnerDirectionalMontageSet* DeathMontageSet = DeathMontages.Find(HitBoneType);
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
