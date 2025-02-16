// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncAction/NexusAsync_WaitForSync.h"

#include "NexusPredictionScope.h"

UNexusAsync_WaitForSync* UNexusAsync_WaitForSync::WaitForSync(UNexusAction* InAction)
{
	UNexusAsync_WaitForSync* SelfObject = NewNexusAsync<UNexusAsync_WaitForSync>(InAction);
	SelfObject->RegisterWithGameInstance(InAction);
	return SelfObject;
}

void UNexusAsync_WaitForSync::Activate()
{
	Super::Activate();

	if (Action->GetActionNetMethod() != ENexusActionNetMethod::LocalPredicted
		|| (Action->IsLocallyControlled() && Action->IsOwnerActorAuthoritative()))
	{
		OnSync();
		return;
	}

	if (Action->IsOwnerActorAuthoritative() && !Action->IsLocallyControlled())
	{
		ActionComponent->CallOrAddSNetyncPointDelegate(Action->GetActionDefinitionHandle(), Action->InitPredictionHandle, FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &UNexusAsync_WaitForSync::OnSync));
		return;
	}

	ActionComponent->CurrentPredictionTag.GenerateNewHandle();
	FNexusPredictionScope ScopedNetPrediction(*ActionComponent, Action->IsOwnerActorAuthoritative(), ActionComponent->CurrentPredictionTag);
	ActionComponent->ServerSendNetSyncPoint(Action->GetActionDefinitionHandle(), Action->InitPredictionHandle, ActionComponent->CurrentPredictionTag);
	OnSync();
}

void UNexusAsync_WaitForSync::OnSync()
{
	if (ShouldBroadcastDelegates() && OnSyncDelegate.IsBound())
	{
		OnSyncDelegate.Broadcast();
		Cancel();
	}
}
