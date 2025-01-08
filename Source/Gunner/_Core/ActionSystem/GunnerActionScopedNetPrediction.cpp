#include "GunnerActionScopedNetPrediction.h"


FGunnerActionScopedNetPrediction::FGunnerActionScopedNetPrediction(UGunnerActionComponent& InActionComponent, bool bInHasAuthority, FGunnerActionNetPredictionHandle InPredictionHandle)
	: ActionComponent(InActionComponent),
	  bHasAuthority(bInHasAuthority)
{
	PrevNetPredictionHandle = ActionComponent.CurrentNetPredictionHandle;
	ActionComponent.CurrentNetPredictionHandle = InPredictionHandle;
}

FGunnerActionScopedNetPrediction::~FGunnerActionScopedNetPrediction()
{
	if (ActionComponent.CurrentNetPredictionHandle.IsValid() && bHasAuthority)
	{
		ActionComponent.GetNetPredictionHandles().ReplicatedNetPredictionHandle(ActionComponent.CurrentNetPredictionHandle);
	}

	ActionComponent.CurrentNetPredictionHandle.Expire();
	if (PrevNetPredictionHandle != ActionComponent.CurrentNetPredictionHandle)
	{
		ActionComponent.CurrentNetPredictionHandle = PrevNetPredictionHandle;
	}
}
