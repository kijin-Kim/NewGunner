#include "NexusPredictionScope.h"


FNexusPredictionScope::FNexusPredictionScope(UNexusActionComponent& InActionComponent, bool bInHasAuthority, FNexusPredictionTag InPredictionTag)
	: ActionComponent(InActionComponent),
	  bHasAuthority(bInHasAuthority)
{
	PrevNetPredictionTag = ActionComponent.CurrentPredictionTag;
	ActionComponent.CurrentPredictionTag = InPredictionTag;
}

FNexusPredictionScope::~FNexusPredictionScope()
{
	if (ActionComponent.CurrentPredictionTag.IsValid() && bHasAuthority)
	{
		ActionComponent.ReplicatedNetPredictionTag(ActionComponent.CurrentPredictionTag);
	}

	ActionComponent.CurrentPredictionTag.Expire();

	if (PrevNetPredictionTag != ActionComponent.CurrentPredictionTag)
	{
		ActionComponent.CurrentPredictionTag = PrevNetPredictionTag;
	}
}
