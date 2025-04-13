#include "NexusPredictionScope.h"
#include "NexusLog.h"


FNexusPredictionScope::FNexusPredictionScope(UNexusPredictionComponent& InPredictionComponent, FNexusPredictionTag InPredictionTag)
	: PredictionComponent(InPredictionComponent),
	  PrevPredictionTag(PredictionComponent.GetCurrentPredictionTag())
{
	PredictionComponent.SetCurrentPredictionTag(InPredictionTag);
	NX_VLOG_SUB(PredictionComponent.GetOwner(), LogNexusPredictionTag, Log, TEXT("예측 구간 [%s] 시작"), *PredictionComponent.GetCurrentPredictionTag().ToString());
}

FNexusPredictionScope::~FNexusPredictionScope()
{
	NX_VLOG_SUB(PredictionComponent.GetOwner(), LogNexusPredictionTag, Log, TEXT("예측 구간 [%s] 종료"), *PredictionComponent.GetCurrentPredictionTag().ToString());
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
