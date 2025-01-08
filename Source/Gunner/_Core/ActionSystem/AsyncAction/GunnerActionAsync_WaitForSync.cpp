// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionAsync_WaitForSync.h"

#include "Gunner/_Core/ActionSystem/GunnerActionScopedNetPrediction.h"

UGunnerActionAsync_WaitForSync* UGunnerActionAsync_WaitForSync::WaitForSync(UGunnerAction* InAction)
{
	UGunnerActionAsync_WaitForSync* SelfObject = NewGunnerAsync<UGunnerActionAsync_WaitForSync>(InAction);
	SelfObject->RegisterWithGameInstance(InAction);
	return SelfObject;
}

void UGunnerActionAsync_WaitForSync::Activate()
{
	Super::Activate();

	if (Action->IsOwnerActorAuthoritative() && !ActionComponent->GetAgentInfo().Pin()->IsLocallyControlled())
	{
		ActionComponent->CallOrAddSNetyncPointDelegate(Action->GetActionDefinitionHandle(), Action->InitPredictionHandle, FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &UGunnerActionAsync_WaitForSync::OnSync));
		return;
	}

	ActionComponent->CurrentNetPredictionHandle.GenerateNewHandle();
	FGunnerActionScopedNetPrediction ScopedNetPrediction(*ActionComponent, Action->IsOwnerActorAuthoritative(), ActionComponent->CurrentNetPredictionHandle);
	ActionComponent->ServerSendNetSyncPoint(Action->GetActionDefinitionHandle(), Action->InitPredictionHandle, ActionComponent->CurrentNetPredictionHandle);
	OnSync();
}

void UGunnerActionAsync_WaitForSync::OnSync()
{
	if (ShouldBroadcastDelegates() && OnSyncDelegate.IsBound())
	{
		OnSyncDelegate.Broadcast();
	}
}
