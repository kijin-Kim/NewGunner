#include "NexusPredictionScope.h"


FNexusPredictionScope::FNexusPredictionScope(UNexusActionComponent& InActionComponent, FNexusPredictionTag InPredictionTag)
	: ActionComponent(InActionComponent),
	  PrevPredictionTag(ActionComponent.CurrentPredictionTag)
{
	ActionComponent.CurrentPredictionTag = InPredictionTag;
}

FNexusPredictionScope::~FNexusPredictionScope()
{
	if (!ActionComponent.IsNetSimulating() && ActionComponent.CurrentPredictionTag.IsValid())
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
