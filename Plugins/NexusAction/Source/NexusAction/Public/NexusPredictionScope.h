#pragma once
#include "NexusActionComponent.h"
#include "NexusPrediction.h"

struct FNexusPredictionScope
{
public:
	FNexusPredictionScope(UNexusActionComponent& InActionComponent, FNexusPredictionTag InPredictionTag);
	~FNexusPredictionScope();

public:
	UNexusActionComponent& ActionComponent;
	FNexusPredictionTag PrevPredictionTag;
};
