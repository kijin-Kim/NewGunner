// Fill out your copyright notice in the Description page of Project Settings.


#include "Action/NexusActionComponent.h"
#include "Action/NexusAction.h"
#include "Action/SubComponent/NexusCueComponent.h"
#include "Action/SubComponent/NexusGameplayTagComponent.h"
#include "Action/SubComponent/NexusPropertyComponent.h"
#include "Action/SubComponent/NexusSideEffectComponent.h"
#include "Cue/NexusCue.h"
#include "Event/NexusEventManagerComponent.h"
#include "SideEffect/NexusSideEffect.h"
#include "SideEffect/NexusSideEffectDef.h"


// ------------------------------------------------------------------------------
// GameplayTag
// ------------------------------------------------------------------------------
void UNexusActionComponent::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	return GetGameplayTagComponent()->GetOwnedGameplayTags(TagContainer);
}

void UNexusActionComponent::PushDynamicTag(const FGameplayTag& Tag)
{
	GetGameplayTagComponent()->PushDynamicTag(Tag);
}

void UNexusActionComponent::PopDynamicTag(const FGameplayTag& Tag)
{
	GetGameplayTagComponent()->PopDynamicTag(Tag);
}


// ------------------------------------------------------------------------------
// SideEffect
// ------------------------------------------------------------------------------
void UNexusActionComponent::BP_TriggerSideEffectToActor(UNexusAction* Action, AActor* SideEffectTarget, TSubclassOf<UNexusSideEffect> SideEffectClass)
{
	check(Action);
	if (!SideEffectTarget)
	{
		return;
	}

	if (UNexusActionComponent* ActionComponent = GetActionComponentFromActor(SideEffectTarget))
	{
		ActionComponent->TriggerSideEffect(SideEffectClass, Action);
	}
}

void UNexusActionComponent::BP_TriggerSideEffectToActorByDef(UNexusAction* Action, AActor* SideEffectTarget, const FNexusSideEffectDef& SideEffectDef)
{
	check(Action);
	if (!SideEffectTarget)
	{
		return;
	}

	if (UNexusActionComponent* ActionComponent = GetActionComponentFromActor(SideEffectTarget))
	{
		ActionComponent->TriggerSideEffectByDef(SideEffectDef, Action);
	}
}

FNexusSideEffectDef UNexusActionComponent::MakeSideEffectDef(UNexusAction* Action, TSubclassOf<UNexusSideEffect> SideEffectClass)
{
	check(Action);
	FNexusSideEffectDef NewSideEffectDef{SideEffectClass};
	NewSideEffectDef.SideEffectInstance = NewObject<UNexusSideEffect>(Action->GetOwnerActor(), SideEffectClass);
	return NewSideEffectDef;
}

void UNexusActionComponent::TriggerSideEffect(TSubclassOf<UNexusSideEffect> SideEffectClass, UNexusAction* Action)
{
	check(Action);
	FNexusSideEffectDef NewSideEffectDef = MakeSideEffectDef(Action, SideEffectClass);
	TriggerSideEffectByDef(NewSideEffectDef, Action);
}

void UNexusActionComponent::TriggerSideEffectByDef(const FNexusSideEffectDef& NewSideEffectDef, UNexusAction* Action, FNexusPredictionEventSignature::FDelegate&& OnPredictionEnded, FNexusPredictionEventSignature::FDelegate&& OnPredictionFailed) const
{
	GetSideEffectComponent()->TriggerSideEffectByDef(NewSideEffectDef, GetCurrentPredictionTag(), MoveTemp(OnPredictionEnded), MoveTemp(OnPredictionFailed));
}


// ------------------------------------------------------------------------------
// Cue
// ------------------------------------------------------------------------------
void UNexusActionComponent::BP_TriggerCue(UNexusAction* Action, TSubclassOf<ANexusCue> CueClass, const FNexusTargetDataHandle& TargetDataHandle)
{
	check(Action);
	AActor* ActorOwner = Cast<AActor>(Action->GetOuter());
	check(ActorOwner);
	if (UNexusActionComponent* ActionComponent = GetActionComponentFromActor(ActorOwner))
	{
		FNexusTriggerCueParams TriggerCueParams;
		TriggerCueParams.CueClass = CueClass;
		TriggerCueParams.TargetDataHandle = TargetDataHandle;
		TriggerCueParams.PredictionTag = ActionComponent->GetCurrentPredictionTag();
		ActionComponent->GetCueComponent()->TriggerCue(TriggerCueParams);
	}
}

void UNexusActionComponent::BP_AuthEndCue(UNexusAction* Action, FNexusLoopingCueHandle CueHandle)
{
	check(Action);
	AActor* ActorOwner = Cast<AActor>(Action->GetOuter());
	check(ActorOwner);
	if (UNexusActionComponent* ActionComponent = GetActionComponentFromActor(ActorOwner))
	{
		ActionComponent->GetCueComponent()->AuthEndCue(CueHandle);
	}
}

void UNexusActionComponent::SimTriggerCue(const FNexusTriggerCueParams& CueParams, FNexusLoopingCueHandle CueHandle)
{
	GetCueComponent()->SimTriggerCue(CueParams, CueHandle);
}


// ------------------------------------------------------------------------------
// Property
// ------------------------------------------------------------------------------

UNexusProperty* UNexusActionComponent::GetProperty(FGameplayTag Tag)
{
	return GetPropertyComponent()->GetProperty(Tag);
}

void UNexusActionComponent::AuthAddProperty(FGameplayTag Tag, float Value)
{
	GetPropertyComponent()->AuthAddProperty(Tag, Value);
}

UNexusProperty* UNexusActionComponent::GetPropertyFromActor(AActor* Actor, FGameplayTag Tag)
{
	if (!Actor)
	{
		return nullptr;
	}

	UNexusActionComponent* ActionComponent = GetActionComponentFromActor(Actor);
	if (!ActionComponent)
	{
		return nullptr;
	}

	return ActionComponent->GetProperty(Tag);
}

float UNexusActionComponent::GetPropertyValueFromActor(AActor* Actor, FGameplayTag Tag)
{
	if (!Actor)
	{
		return 0.0f;
	}

	UNexusActionComponent* ActionComponent = GetActionComponentFromActor(Actor);
	if (!ActionComponent)
	{
		return 0.0f;
	}

	UNexusProperty* PropertyPtr = ActionComponent->GetProperty(Tag);
	if (!PropertyPtr)
	{
		return 0.0f;
	}

	return PropertyPtr->GetDynamicValue();
}

void UNexusActionComponent::AddStaticOperation(FGameplayTag Tag, FNexusPropertyOperation Operation)
{
	GetPropertyComponent()->AddStaticOperation(Tag, Operation);
}

void UNexusActionComponent::AddDynamicOperation(FGameplayTag Tag, FNexusPropertyOperation Operation)
{
	GetPropertyComponent()->AddDynamicOperation(Tag, Operation);
}

void UNexusActionComponent::RemoveOperationByHandle(FGameplayTag Tag, const FNexusPropertyOperationHandle& OperationHandle)
{
	GetPropertyComponent()->RemoveOperationByHandle(Tag, OperationHandle);
}


// ------------------------------------------------------------------------------
// Event
// ------------------------------------------------------------------------------
void UNexusActionComponent::HandleEvent(FGameplayTag EventTag, const void* Message, UScriptStruct* MessageType)
{
	EventManagerComponent->HandleEvent(EventTag, Message, MessageType);
}

DEFINE_FUNCTION(UNexusActionComponent::execBP_SendEventToActor)
{
	P_GET_STRUCT(FGameplayTag, EventTag);
	P_GET_OBJECT(AActor, TargetActor);
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
	void* MessagePtr = Stack.MostRecentPropertyAddress;


	P_FINISH;

	if (!EventTag.IsValid() || !TargetActor || !MessagePtr)
	{
		return;
	}

	if (UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(TargetActor))
	{
		ActionComponent->HandleEvent(EventTag, MessagePtr, StructProperty->Struct);
	}
}

FNexusEventCallbackHandle UNexusActionComponent::BindEventCallbackDirect(FGameplayTag EventTag, TFunction<void(FGameplayTag, const void*)>&& Callbacks, UScriptStruct* MessageType) const
{
	return EventManagerComponent->BindEventCallbackDirect(EventTag, MoveTemp(Callbacks), MessageType);
}

void UNexusActionComponent::UnbindEventCallback(FNexusEventCallbackHandle Handle) const
{
	EventManagerComponent->UnbindEventCallback(Handle);
}

UNexusCueComponent* UNexusActionComponent::GetCueComponent() const
{
	return GetCachedComponent(CueComponentCached);
}

UNexusPredictionComponent* UNexusActionComponent::GetPredictionComponent() const
{
	return GetCachedComponent(PredictionComponentCached);
}

UNexusSideEffectComponent* UNexusActionComponent::GetSideEffectComponent() const
{
	return GetCachedComponent(SideEffectComponentCached);
}

UNexusPropertyComponent* UNexusActionComponent::GetPropertyComponent() const
{
	return GetCachedComponent(PropertyComponentCached);
}

UNexusGameplayTagComponent* UNexusActionComponent::GetGameplayTagComponent() const
{
	return GetCachedComponent(GameplayTagComponentCached);
}

UNexusEventManagerComponent* UNexusActionComponent::GetEventMangerComponent() const
{
	return EventManagerComponent;
}
