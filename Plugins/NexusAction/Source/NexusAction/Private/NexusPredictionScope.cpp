#include "NexusPredictionScope.h"
#include "NexusLog.h"


FNexusPredictionScope::FNexusPredictionScope(UNexusActionComponent& InActionComponent, FNexusPredictionTag InPredictionTag)
	: ActionComponent(InActionComponent),
	  PrevPredictionTag(ActionComponent.CurrentPredictionTag)
{
	ActionComponent.CurrentPredictionTag = InPredictionTag;
	NX_VLOG_SUB(ActionComponent.GetOwner(), LogNexusPredictionTag, Log, TEXT("예측 구간 [%s] 시작"), *ActionComponent.CurrentPredictionTag.ToString());
}

FNexusPredictionScope::~FNexusPredictionScope()
{
	NX_VLOG_SUB(ActionComponent.GetOwner(), LogNexusPredictionTag, Log, TEXT("예측 구간 [%s] 종료"), *ActionComponent.CurrentPredictionTag.ToString());
	if (!ActionComponent.IsNetSimulating() && ActionComponent.CurrentPredictionTag.IsValid())
	{
		ActionComponent.ReplicateNetPredictionTag(ActionComponent.CurrentPredictionTag);
	}

	if (PrevPredictionTag != ActionComponent.CurrentPredictionTag)
	{
		ActionComponent.CurrentPredictionTag = PrevPredictionTag;
	}
	else
	{
		ActionComponent.CurrentPredictionTag = FNexusPredictionTag();
	}
}
