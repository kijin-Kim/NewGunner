#include "Prediction/NexusPredictionScope.h"
#include "NexusLog.h"
#include "Action/SubComponent/NexusPredictionComponent.h"


FNexusPredictionScope::FNexusPredictionScope(UNexusPredictionComponent& InPredictionComponent, FNexusPredictionTag InPredictionTag, const FString& InPredictionScopeName)
	: PredictionComponent(InPredictionComponent),
	  PrevPredictionTag(PredictionComponent.GetCurrentPredictionTag()),
	  PredictionScopeName( InPredictionScopeName)
{
	PredictionComponent.SetCurrentPredictionTag(InPredictionTag);
	NX_VLOG_SUB(PredictionComponent.GetAgentActor(), LogNexusPrediction, VeryVerbose, TEXT("예측 구간 시작: Name=%s, %s"), *PredictionScopeName, *PredictionComponent.GetCurrentPredictionTag().ToString());
}

FNexusPredictionScope::~FNexusPredictionScope()
{
	NX_VLOG_SUB(PredictionComponent.GetAgentActor(), LogNexusPrediction, VeryVerbose, TEXT("예측 구간 종료: Name=%s, %s"), *PredictionScopeName, *PredictionComponent.GetCurrentPredictionTag().ToString());
	if (!PredictionComponent.IsNetSimulating() && PredictionComponent.GetCurrentPredictionTag().IsValid())
	{
		PredictionComponent.ReplicateNetPredictionTag(PredictionComponent.GetCurrentPredictionTag());
	}

	if (PrevPredictionTag != PredictionComponent.GetCurrentPredictionTag())
	{
		PredictionComponent.SetCurrentPredictionTag(PrevPredictionTag);
	}
	else
	{
		PredictionComponent.SetCurrentPredictionTag(FNexusPredictionTag());
	}
}

UObject* FNexusPredictionScope::GetOuter() const
{
	return PredictionComponent.GetOwner();
}
