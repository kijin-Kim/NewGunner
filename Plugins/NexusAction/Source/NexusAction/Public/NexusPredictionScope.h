#pragma once
#include "NexusActionComponent.h"
#include "NexusPrediction.h"

struct FNexusPredictionScope
{
public:
	FNexusPredictionScope(UNexusPredictionComponent& InPredictionComponent, FNexusPredictionTag InPredictionTag);
	~FNexusPredictionScope();

private:
	// 로깅을 위한 함수
	UObject* GetOuter() const
	{
		return PredictionComponent.GetOwner();
	}

public:
	UNexusPredictionComponent& PredictionComponent;
	FNexusPredictionTag PrevPredictionTag;
};
