// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionAsync_WaitForGunnerEvent.h"

#include "Gunner/_Core/ActionSystem/GunnerActionScopedNetPrediction.h"
#include "Gunner/_Core/Event/GunnerEventManagerComponent.h"

UGunnerActionAsync_WaitForGunnerEvent* UGunnerActionAsync_WaitForGunnerEvent::WaitForGunnerEvent(UGunnerAction* InAction, AActor* EventTargetActor, FGameplayTag InEventTag, bool bInReplicates, UScriptStruct* InEventMessageType)
{
	UGunnerActionAsync_WaitForGunnerEvent* SelfObject = NewGunnerAsync<UGunnerActionAsync_WaitForGunnerEvent>(InAction);
	if (!EventTargetActor || !InEventTag.IsValid() || !InEventMessageType)
	{
		return nullptr;
	}

	SelfObject->TargetEventManagerComponent = UGunnerEventManagerComponent::GetEventManagerComponentFromActor(EventTargetActor);
	if (!SelfObject->TargetEventManagerComponent.IsValid())
	{
		return nullptr;
	}

	SelfObject->EventTag = InEventTag;
	SelfObject->EventMesageType = InEventMessageType;
	SelfObject->bReplciates = bInReplicates;
	SelfObject->RegisterWithGameInstance(EventTargetActor->GetWorld());

	return SelfObject;
}

void UGunnerActionAsync_WaitForGunnerEvent::OnSync()
{
	OnEventReceivedDelegate.Broadcast(EventTag);
	Cancel();
}

void UGunnerActionAsync_WaitForGunnerEvent::Activate()
{
	Super::Activate();

	if (bReplciates)
	{
		if (Action->IsOwnerActorAuthoritative() && !ActionComponent->GetAgentInfo().Pin()->IsLocallyControlled())
		{
			ActionComponent->CallOrAddSNetyncPointDelegate(Action->GetActionDefinitionHandle(), Action->InitPredictionHandle, FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &UGunnerActionAsync_WaitForGunnerEvent::OnSync));
			return;
		}
	}

	UnbindEvents(TargetEventManagerComponent.Get());
	BindEvents();
}

void UGunnerActionAsync_WaitForGunnerEvent::SetReadyToDestroy()
{
	Super::SetReadyToDestroy();
	MessagePtr = nullptr;
	UnbindEvents(TargetEventManagerComponent.Get());
}

TArray<FGunnerEventCallbackHandle> UGunnerActionAsync_WaitForGunnerEvent::SetupEvents()
{
	TWeakObjectPtr<UGunnerActionAsync_WaitForGunnerEvent> Weak = this;
	return {
		TargetEventManagerComponent->BindEventCallbackInternal(EventTag, [Weak, this](FGameplayTag Tag, const void* MessagePtr)
		{
			if (bReplciates)
			{
				ActionComponent->CurrentNetPredictionHandle.GenerateNewHandle();
				ActionComponent->ServerSendNetSyncPoint(Action->GetActionDefinitionHandle(), Action->InitPredictionHandle, ActionComponent->CurrentNetPredictionHandle);
			}
			FGunnerActionScopedNetPrediction ScopedNetPrediction(*ActionComponent, Action->IsOwnerActorAuthoritative(), ActionComponent->CurrentNetPredictionHandle);
			
			UGunnerActionAsync_WaitForGunnerEvent* Strong = Weak.Get();
			if (Strong && Strong->ShouldBroadcastDelegates())
			{
				Strong->MessagePtr = MessagePtr;
				Strong->OnEventReceivedDelegate.Broadcast(Tag);
				Strong->Cancel();
			}
		}, EventMesageType.Get())
	};
}

DEFINE_FUNCTION(UGunnerActionAsync_WaitForGunnerEvent::execGetMessage)
{
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* OutMessagePtr = Stack.MostRecentPropertyAddress;
	FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
	P_FINISH;

	if (!P_THIS->MessagePtr)
	{
		*StaticCast<bool*>(RESULT_PARAM) = false;
		return;
	}

	if (!StructProperty || !StructProperty->Struct || !OutMessagePtr)
	{
		*StaticCast<bool*>(RESULT_PARAM) = false;
		return;
	}

	if (StructProperty->Struct != P_THIS->EventMesageType.Get())
	{
		*StaticCast<bool*>(RESULT_PARAM) = false;
		return;
	}

	StructProperty->Struct->CopyScriptStruct(OutMessagePtr, P_THIS->MessagePtr);
	*StaticCast<bool*>(RESULT_PARAM) = true;
}
