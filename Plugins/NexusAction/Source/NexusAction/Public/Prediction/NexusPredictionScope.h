#pragma once
#include "Action/NexusActionComponent.h"
#include "NexusPrediction.h"

struct FNexusPredictionScope
{
public:
	FNexusPredictionScope(UNexusPredictionComponent& InPredictionComponent, FNexusPredictionTag InPredictionTag, const FString& InPredictionScopeName);
	~FNexusPredictionScope();

private:
	// 로깅을 위한 함수
	UObject* GetOuter() const;

public:
	UNexusPredictionComponent& PredictionComponent;
	FNexusPredictionTag PrevPredictionTag;
	FString PredictionScopeName;
};
