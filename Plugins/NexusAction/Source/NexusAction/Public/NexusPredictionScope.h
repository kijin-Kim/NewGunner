#pragma once
#include "NexusActionComponent.h"
#include "NexusPrediction.h"

struct FNexusPredictionScope
{
public:
	FNexusPredictionScope(UNexusActionComponent& InActionComponent, bool bInHasAuthority, FNexusPredictionTag InPredictionTag);
	~FNexusPredictionScope();

public:
	UNexusActionComponent& ActionComponent;
	bool bHasAuthority;
	FNexusPredictionTag PrevNetPredictionTag;
	
};
