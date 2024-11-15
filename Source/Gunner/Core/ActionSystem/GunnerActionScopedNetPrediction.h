#pragma once
#include "GunnerActionComponent.h"
#include "GunnerActionDefinition.h"
#include "AsyncAction/GunnerActionNetPrediction.h"

struct FGunnerActionScopedNetPrediction
{
public:
	FGunnerActionScopedNetPrediction(UGunnerActionComponent& InActionComponent, bool bInHasAuthority, FGunnerActionNetPredictionHandle InPredictionHandle);
	~FGunnerActionScopedNetPrediction();

public:
	UGunnerActionComponent& ActionComponent;
	bool bHasAuthority;
	FGunnerActionNetPredictionHandle PrevNetPredictionHandle;
	
};
