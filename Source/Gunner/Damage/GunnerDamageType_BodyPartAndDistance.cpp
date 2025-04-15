// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerDamageType_BodyPartAndDistance.h"

#include "Gunner/_Core/Damage/GunnerDamageContext.h"
#include "Gunner/_Core/GunnerHitBoxInterface.h"
#include "Misc/DataValidation.h"

#if WITH_EDITOR
EDataValidationResult UGunnerDamageType_BodyPartAndDistance::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	float LastStartDistance = -1.0f;
	for (const auto& [StartDistance, Multiplier] : DistanceDamageFallOffs)
	{
		if (StartDistance <= LastStartDistance)
		{
			FString Message = FString::Printf(TEXT("상한 거리는 이전 상한 거리보다 커야 합니다."));
			Context.AddError(FText::FromString(Message));
			return EDataValidationResult::Invalid;
		}
		LastStartDistance = StartDistance;
	}
	return Result;
}
#endif

float UGunnerDamageType_BodyPartAndDistance::CalculateDamageByContext(UGunnerDamageContext* DamageContext) const
{
	check(DamageContext);
	return Super::CalculateDamageByContext(DamageContext) * GetBodyPartMultiplier(DamageContext) * GetDistanceFallOffMultiplier(DamageContext);
}

float UGunnerDamageType_BodyPartAndDistance::GetDistanceFallOffMultiplier(UGunnerDamageContext* DamageContext) const
{
	check(DamageContext);
	if (DistanceDamageFallOffs.IsEmpty())
	{
		return 1.0f;
	}

	const FVector InstigatorLocation = DamageContext->Instigator->GetPawn()->GetActorLocation();
	const FVector TargetLocation = DamageContext->Target->GetActorLocation();
	const float Distance = FVector::Dist(TargetLocation, InstigatorLocation);

	float LastMultiplier = 1.0f;
	for (const FDistanceDamageFallOff& FallOff : DistanceDamageFallOffs)
	{
		if (Distance < FallOff.StartDistance)
		{
			return LastMultiplier;
		}
		LastMultiplier = FallOff.Multiplier;
	}
	return LastMultiplier;
}

float UGunnerDamageType_BodyPartAndDistance::GetBodyPartMultiplier(UGunnerDamageContext* DamageContext) const
{
	check(DamageContext);
	
	if (!DamageContext->Target)
	{
		return 1.0f;
	}

	EGunnerHitBoxType HitBoxType = EGunnerHitBoxType::Body;
	if (DamageContext->Target->Implements<UGunnerHitBoxInterface>())
	{
		HitBoxType = IGunnerHitBoxInterface::Execute_GetHitBoxTypeByHitBoneName(DamageContext->Target, DamageContext->HitBoneName);
	}

	switch (HitBoxType)
	{
	case EGunnerHitBoxType::Head:
		return HeadDamageMultiplier;
	case EGunnerHitBoxType::Leg:
		return LegDamageMultiplier;
	default:
		return 1.0f;
	}
}
