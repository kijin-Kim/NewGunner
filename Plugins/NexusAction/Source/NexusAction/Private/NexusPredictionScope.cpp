#include "NexusPredictionScope.h"


FNexusPredictionScope::FNexusPredictionScope(UNexusActionComponent& InActionComponent, bool bInHasAuthority, FNexusPredictionTag InPredictionHandle)
	: ActionComponent(InActionComponent),
	  bHasAuthority(bInHasAuthority)
{
	PrevNetPredictionHandle = ActionComponent.CurrentPredictionTag;
	ActionComponent.CurrentPredictionTag = InPredictionHandle;
}

FNexusPredictionScope::~FNexusPredictionScope()
{
	if (ActionComponent.CurrentPredictionTag.IsValid() && bHasAuthority)
	{
		ActionComponent.ReplicatedNetPredictionHandle(ActionComponent.CurrentPredictionTag);
	}

	ActionComponent.CurrentPredictionTag.Expire();

	if (PrevNetPredictionHandle != ActionComponent.CurrentPredictionTag)
	{
		ActionComponent.CurrentPredictionTag = PrevNetPredictionHandle;
	}
}
