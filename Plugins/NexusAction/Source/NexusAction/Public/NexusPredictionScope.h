#pragma once
#include "NexusActionComponent.h"
#include "NexusPrediction.h"

struct FNexusPredictionScope
{
public:
	FNexusPredictionScope(UNexusActionComponent& InActionComponent, FNexusPredictionTag InPredictionTag, bool bInIsAlreadyReplicated);
	~FNexusPredictionScope();

public:
	UNexusActionComponent& ActionComponent;
	FNexusPredictionTag PrevPredictionTag;
	bool bIsAlreadyReplicated;
};
