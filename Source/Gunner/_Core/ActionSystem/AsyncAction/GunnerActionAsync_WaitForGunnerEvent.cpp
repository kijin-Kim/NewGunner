// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionAsync_WaitForGunnerEvent.h"
#include "Gunner/_Core/Event/GunnerEventManagerComponent.h"

UGunnerActionAsync_WaitForGunnerEvent* UGunnerActionAsync_WaitForGunnerEvent::WaitForGunnerEvent(UGunnerAction* InAction, AActor* EventTargetActor, FGameplayTag InEventTag, UScriptStruct* InEventMessageType)
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
	SelfObject->RegisterWithGameInstance(EventTargetActor->GetWorld());

	return SelfObject;
}

void UGunnerActionAsync_WaitForGunnerEvent::Activate()
{
	Super::Activate();
	UnbindEvents();
	BindEvents();
}

void UGunnerActionAsync_WaitForGunnerEvent::SetReadyToDestroy()
{
	Super::SetReadyToDestroy();
	MessagePtr = nullptr;
	UnbindEvents();
}

TArray<FGunnerEventCallbackHandle> UGunnerActionAsync_WaitForGunnerEvent::SetupEvents()
{
	TWeakObjectPtr<UGunnerActionAsync_WaitForGunnerEvent> Weak = this;
	return {
		TargetEventManagerComponent->BindEventCallbackInternal(EventTag, [Weak](FGameplayTag Tag, const void* MessagePtr)
		{
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

UGunnerEventManagerComponent* UGunnerActionAsync_WaitForGunnerEvent::GetEventManagerComponent() const
{
	return TargetEventManagerComponent.Get();
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
