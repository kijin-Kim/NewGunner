// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncAction/NexusAsync_SendOrWaitTargetData.h"
#include "Prediction/NexusPredictionScope.h"


UNexusAsync_SendOrWaitTargetData* UNexusAsync_SendOrWaitTargetData::SendOrWaitTargetData(UNexusAction* InAction, const FNexusTargetDataHandle& InTargetDataHandle)
{
	UNexusAsync_SendOrWaitTargetData* SelfObject = NewNexusAsync<UNexusAsync_SendOrWaitTargetData>(InAction);
	SelfObject->RegisterWithGameInstance(InAction);
	SelfObject->TargetDataHandle = InTargetDataHandle;
	return SelfObject;
}

void UNexusAsync_SendOrWaitTargetData::Activate()
{
	Super::Activate();

	bool bIsOwnerActorAuthoritative = Action->IsOwnerActorAuthoritative();
	FNexusPredictionTag PredictionTag;
	PredictionTag.GenerateNewHandle(bIsOwnerActorAuthoritative);
	UNexusPredictionComponent* PredictionComponent = Action->GetOwnerActor()->GetComponentByClass<UNexusPredictionComponent>();
	check(PredictionComponent);
	FNexusPredictionScope PredictionScope(*PredictionComponent, PredictionTag);


	if (Action->GetActionNetMethod() != ENexusActionNetMethod::LocalPredicted
		|| (Action->IsLocallyControlled() && bIsOwnerActorAuthoritative))
	{
		OnArrived(TargetDataHandle);
		return;
	}

	if (bIsOwnerActorAuthoritative && !Action->IsLocallyControlled())
	{
		PredictionComponent->CallOrAddTargetDataDelegate(Action->GetActionDefHandle(), Action->GetPrimaryPredictionTag(), FOnNexusTargetDataSetSignature::FDelegate::CreateUObject(this, &UNexusAsync_SendOrWaitTargetData::OnArrived));
		return;
	}

	check(TargetDataHandle.IsValid());
	PredictionComponent->ServerSendTargetData(Action->GetActionDefHandle(), Action->GetPrimaryPredictionTag(), PredictionComponent->GetCurrentPredictionTag(), TargetDataHandle);
	OnArrived(TargetDataHandle);
}

void UNexusAsync_SendOrWaitTargetData::OnArrived(FNexusTargetDataHandle InTargetDataHandle)
{
	check(InTargetDataHandle.IsValid());
	if (ShouldBroadcastDelegates() && OnArrivedDelegate.IsBound())
	{
		OnArrivedDelegate.Broadcast(InTargetDataHandle);
		Cancel();
	}
}
