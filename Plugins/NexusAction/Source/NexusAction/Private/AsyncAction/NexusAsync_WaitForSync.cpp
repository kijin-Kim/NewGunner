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

	bool bIsOwnerActorAuthoritative = Action->IsOwnerActorAuthoritative();
	FNexusPredictionTag PredictionTag;
	PredictionTag.GenerateNewHandle(bIsOwnerActorAuthoritative);
	UNexusPredictionComponent* PredictionComponent = Action->GetOwnerActor()->GetComponentByClass<UNexusPredictionComponent>();
	FNexusPredictionScope PredictionScope(*PredictionComponent, PredictionTag);


	if (Action->GetActionNetMethod() != ENexusActionNetMethod::LocalPredicted
		|| (Action->IsLocallyControlled() && bIsOwnerActorAuthoritative))
	{
		OnSync();
		return;
	}

	if (bIsOwnerActorAuthoritative && !Action->IsLocallyControlled())
	{
		PredictionComponent->CallOrAddNetsyncPointDelegate(Action->GetActionDefHandle(), Action->GetPrimaryPredictionTag(), FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &UNexusAsync_WaitForSync::OnSync));
		return;
	}

	PredictionComponent->ServerSendNetSyncPoint(Action->GetActionDefHandle(), Action->GetPrimaryPredictionTag(), PredictionComponent->GetCurrentPredictionTag());
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
