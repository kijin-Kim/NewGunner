// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerHitBoxInterface.h"


// Add default functionality here for any IGunnerHitBoxInterface functions that are not pure virtual.
EGunnerHitDirectionType IGunnerHitBoxInterface::GetHitDirectionType(const FVector& CauserLocation, const FVector& VictimLocation, const FVector& VictimForward)
{
	const FVector ToCauser = (CauserLocation - VictimLocation).GetSafeNormal2D();
	
	const float DotFront = ToCauser.Dot(VictimForward);
	const float CrossZ = VictimForward.Cross(ToCauser).Z;

	if (abs(DotFront) > abs(CrossZ))
	{
		return DotFront > 0.0f ? EGunnerHitDirectionType::Front : EGunnerHitDirectionType::Back;
	}

	return CrossZ > 0.0f ? EGunnerHitDirectionType::Right : EGunnerHitDirectionType::Left;
}
