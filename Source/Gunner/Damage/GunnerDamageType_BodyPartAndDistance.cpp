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

float UGunnerDamageType_BodyPartAndDistance::CalculateDamageByContext(const FGunnerDamageContext& DamageContext) const
{
	
	return Super::CalculateDamageByContext(DamageContext) * GetBodyPartMultiplier(DamageContext) * GetDistanceFallOffMultiplier(DamageContext);
}

float UGunnerDamageType_BodyPartAndDistance::GetDistanceFallOffMultiplier(const FGunnerDamageContext& DamageContext) const
{
	if (DistanceDamageFallOffs.IsEmpty())
	{
		return 1.0f;
	}

	const FVector InstigatorLocation = DamageContext.Instigator->GetActorLocation();
	const FVector TargetLocation = DamageContext.Target->GetActorLocation();
	const float Distance = FVector::Dist(TargetLocation, InstigatorLocation) * 0.01f; // m로 변환

	float LastMultiplier = 1.0f;
	for (const FDistanceDamageFallOff& FallOff : DistanceDamageFallOffs)
	{
		if (Distance < FallOff.StartDistanceMeter)
		{
			return LastMultiplier;
		}
		LastMultiplier = FallOff.Multiplier;
	}
	return LastMultiplier;
}

float UGunnerDamageType_BodyPartAndDistance::GetBodyPartMultiplier(const FGunnerDamageContext& DamageContext) const
{
	
	if (!DamageContext.Target)
	{
		return 1.0f;
	}

	EGunnerHitPartType HitBoxType = EGunnerHitPartType::Body;
	if (DamageContext.Target->Implements<UGunnerHitBoxInterface>())
	{
		HitBoxType = IGunnerHitBoxInterface::Execute_GetHitPartTypeByHitBoneName(DamageContext.Target, DamageContext.HitBoneName);
	}

	switch (HitBoxType)
	{
	case EGunnerHitPartType::Head:
		return HeadDamageMultiplier;
	case EGunnerHitPartType::Leg:
		return LegDamageMultiplier;
	default:
		return 1.0f;
	}
}
