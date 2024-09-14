// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncAction_WaitForGunnerEvent.h"
#include "EventManagerComponent.h"

UAsyncAction_WaitForGunnerEvent* UAsyncAction_WaitForGunnerEvent::WaitForGunnerEvent(AActor* EventTargetActor, FGameplayTag InEventTag, UScriptStruct* InEventMessageType)
{
	UAsyncAction_WaitForGunnerEvent* Action = NewObject<UAsyncAction_WaitForGunnerEvent>();
	if (!EventTargetActor || !InEventTag.IsValid() || !InEventMessageType)
	{
		return nullptr;
	}

	Action->TargetEventManagerComponent = EventTargetActor->GetComponentByClass<UEventManagerComponent>();
	if (!Action->TargetEventManagerComponent.IsValid())
	{
		return nullptr;
	}

	Action->EventTag = InEventTag;
	Action->EventMesageType = InEventMessageType;

	return Action;
}

void UAsyncAction_WaitForGunnerEvent::Activate()
{
	Super::Activate();
	UnbindEvents();
	BindEvents();
}

void UAsyncAction_WaitForGunnerEvent::SetReadyToDestroy()
{
	Super::SetReadyToDestroy();
	MessagePtr = nullptr;
	UnbindEvents();
}

TArray<FEventCallbackHandle> UAsyncAction_WaitForGunnerEvent::SetupEvents()
{
	TWeakObjectPtr<UAsyncAction_WaitForGunnerEvent> Weak = this;
	return {
		TargetEventManagerComponent->BindEventCallbackInternal(EventTag, [Weak](FGameplayTag Tag, const void* MessagePtr)
		{
			if (UAsyncAction_WaitForGunnerEvent* Strong = Weak.Get())
			{
				Strong->MessagePtr = MessagePtr;
				Strong->OnEventReceived.Broadcast(Tag);
			}
		})
	};
}

UEventManagerComponent* UAsyncAction_WaitForGunnerEvent::GetEventManagerComponent() const
{
	return TargetEventManagerComponent.Get();
}

DEFINE_FUNCTION(UAsyncAction_WaitForGunnerEvent::execGetMessage)
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
