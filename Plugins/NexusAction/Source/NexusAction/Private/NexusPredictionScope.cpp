#include "NexusPredictionScope.h"


FNexusPredictionScope::FNexusPredictionScope(UNexusActionComponent& InActionComponent, FNexusPredictionTag InPredictionTag, bool bInIsAlreadyReplicated)
	: ActionComponent(InActionComponent),
	  PrevPredictionTag(ActionComponent.CurrentPredictionTag),
	  bIsAlreadyReplicated(bInIsAlreadyReplicated)
{
	ActionComponent.CurrentPredictionTag = InPredictionTag;
}

FNexusPredictionScope::~FNexusPredictionScope()
{
	if (!bIsAlreadyReplicated && ActionComponent.IsNetSimulating() && ActionComponent.CurrentPredictionTag.IsPredictable())
	{
		ActionComponent.ReplicatedNetPredictionTag(ActionComponent.CurrentPredictionTag);
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
