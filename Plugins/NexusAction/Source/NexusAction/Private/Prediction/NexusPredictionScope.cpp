#include "Prediction/NexusPredictionScope.h"
#include "NexusLog.h"
#include "Action/SubComponent/NexusPredictionComponent.h"


FNexusPredictionScope::FNexusPredictionScope(UNexusPredictionComponent& InPredictionComponent, FNexusPredictionTag InPredictionTag)
	: PredictionComponent(InPredictionComponent),
	  PrevPredictionTag(PredictionComponent.GetCurrentPredictionTag())
{
	PredictionComponent.SetCurrentPredictionTag(InPredictionTag);
	NX_VLOG_SUB(PredictionComponent.GetAgentActor(), LogNexusPrediction, VeryVerbose, TEXT("예측 구간 시작: %s"), *PredictionComponent.GetCurrentPredictionTag().ToString());
}

FNexusPredictionScope::~FNexusPredictionScope()
{
	NX_VLOG_SUB(PredictionComponent.GetAgentActor(), LogNexusPrediction, VeryVerbose, TEXT("예측 구간 종료: %s"), *PredictionComponent.GetCurrentPredictionTag().ToString());
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
