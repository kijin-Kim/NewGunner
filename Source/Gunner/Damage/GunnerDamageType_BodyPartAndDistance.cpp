// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerDamageType_BodyPartAndDistance.h"
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

float UGunnerDamageType_BodyPartAndDistance::CalculateDamageByContext(const FDamageContext& DamageContext) const
{
	check(DamageContext.IsValid());
	return Super::CalculateDamageByContext(DamageContext) * GetBodyPartMultiplier(DamageContext) * GetDistanceFallOffMultiplier(DamageContext);
}

float UGunnerDamageType_BodyPartAndDistance::GetDistanceFallOffMultiplier(const FDamageContext& DamageContext) const
{
	check(DamageContext.IsValid());
	if (DistanceDamageFallOffs.IsEmpty())
	{
		return 1.0f;
	}

	const FVector InstigatorLocation = DamageContext.Instigator->GetPawn()->GetActorLocation();
	const FVector TargetLocation = DamageContext.Target->GetActorLocation();
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

float UGunnerDamageType_BodyPartAndDistance::GetBodyPartMultiplier(const FDamageContext& DamageContext) const
{
	// TODO: Instigator->GetBodyType() == Humonoid
	check(DamageContext.IsValid());
	return 0.0f;
}