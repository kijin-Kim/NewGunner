#pragma once
#include "NexusActionComponent.h"
#include "NexusPrediction.h"

struct FNexusPredictionScope
{
public:
	FNexusPredictionScope(UNexusActionComponent& InActionComponent, FNexusPredictionTag InPredictionTag);
	~FNexusPredictionScope();

private:
	// 로깅을 위한 함수
	UObject* GetOuter() const
	{
		return ActionComponent.GetOwner();
	}

public:
	UNexusActionComponent& ActionComponent;
	FNexusPredictionTag PrevPredictionTag;
};
