#include "GunnerActionScopedNetPrediction.h"


FGunnerActionScopedNetPrediction::FGunnerActionScopedNetPrediction(UGunnerActionComponent& InActionComponent, bool bInHasAuthority, FGunnerActionNetPredictionHandle InPredictionHandle)
	: ActionComponent(InActionComponent),
	  bHasAuthority(bInHasAuthority)
{
	PrevNetPredictionHandle = ActionComponent.NetPredictionHandle;
	ActionComponent.NetPredictionHandle = InPredictionHandle;
}

FGunnerActionScopedNetPrediction::~FGunnerActionScopedNetPrediction()
{
	if (ActionComponent.NetPredictionHandle.IsValid() && bHasAuthority)
	{
		ActionComponent.GetNetPredictionHandleArray().ReplicatedNetPredictionHandle(ActionComponent.NetPredictionHandle);
	}

	ActionComponent.NetPredictionHandle.Expire();
	if (PrevNetPredictionHandle != ActionComponent.NetPredictionHandle)
	{
		ActionComponent.NetPredictionHandle = PrevNetPredictionHandle;
	}
}
