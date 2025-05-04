// Fill out your copyright notice in the Description page of Project Settings.


#include "Action/NexusActionComponent.h"
#include "Action/NexusAction.h"
#include "Action/SubComponent/NexusCueComponent.h"
#include "Action/SubComponent/NexusGameplayTagComponent.h"
#include "Action/SubComponent/NexusPropertyComponent.h"
#include "Action/SubComponent/NexusSideEffectComponent.h"
#include "Action/SubComponent/NexusEventManagerComponent.h"
#include "Cue/NexusCue.h"
#include "SideEffect/NexusSideEffect.h"
#include "SideEffect/NexusSideEffectInstance.h"


// ------------------------------------------------------------------------------
// SideEffect
// ------------------------------------------------------------------------------
void UNexusActionComponent::BP_ApplySideEffectToActor(UNexusAction* Action, AActor* SideEffectTarget, TSubclassOf<UNexusSideEffect> SideEffectClass)
{
	check(Action);
	if (!SideEffectTarget)
	{
		return;
	}

	if (UNexusActionComponent* ActionComponent = GetActionComponentFromActor(SideEffectTarget))
	{
		ActionComponent->ApplySideEffect(SideEffectClass, Action);
	}
}

void UNexusActionComponent::BP_ApplySideEffectToActorByDef(UNexusAction* Action, AActor* SideEffectTarget, const FNexusSideEffectInstanceDefHandle& SideEffectInstanceDefHandle)
{
	check(Action);
	if (!SideEffectTarget)
	{
		return;
	}

	if (UNexusActionComponent* ActionComponent = GetActionComponentFromActor(SideEffectTarget))
	{
		ActionComponent->ApplySideEffectByDef(*SideEffectInstanceDefHandle.GetData());
	}
}

FNexusSideEffectInstanceDefHandle UNexusActionComponent::MakeSideEffectInstanceDef(TSubclassOf<UNexusSideEffect> SideEffectClass)
{
	// Blueprint에서 복사방지 및 스테이트를 유지하기 위한 핸들 패턴
	FNexusSideEffectInstanceDefHandle Handle;
	Handle.SetData(MakeShared<FNexusSideEffectInstanceDef>(SideEffectClass));
	return Handle;
}

FNexusSideEffectInstanceHandle UNexusActionComponent::ApplySideEffect(TSubclassOf<UNexusSideEffect> SideEffectClass, UNexusAction* Action)
{
	check(Action);
	FNexusSideEffectInstanceDef SideEffectInstanceDef{SideEffectClass};
	return ApplySideEffectByDef(SideEffectInstanceDef);
}

FNexusSideEffectInstanceHandle UNexusActionComponent::ApplySideEffectByDef(const FNexusSideEffectInstanceDef& SideEffectInstanceDef, FNexusPredictionEventSignature::FDelegate&& OnPredictionEnded, FNexusPredictionEventSignature::FDelegate&& OnPredictionFailed) const
{
	return GetSideEffectComponent()->ApplySideEffectByDef(SideEffectInstanceDef, GetCurrentPredictionTag(), MoveTemp(OnPredictionEnded), MoveTemp(OnPredictionFailed));
}


// ------------------------------------------------------------------------------
// Property
// ------------------------------------------------------------------------------

UNexusProperty* UNexusActionComponent::GetProperty(FGameplayTag Tag)
{
	return GetPropertyComponent()->GetProperty(Tag);
}

float UNexusActionComponent::GetPropertyValue(FGameplayTag Tag)
{
	return GetPropertyComponent()->GetPropertyValue(Tag);
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

	return ActionComponent->GetPropertyValue(Tag);
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

void UNexusActionComponent::PushStaticTag(const FGameplayTag& Tag)
{
	GetGameplayTagComponent()->PushStaticTag(Tag);
}

void UNexusActionComponent::PopStaticTag(const FGameplayTag& Tag)
{
	GetGameplayTagComponent()->PopStaticTag(Tag);
}


// ------------------------------------------------------------------------------
// Cue
// ------------------------------------------------------------------------------
void UNexusActionComponent::BP_TriggerCue(UNexusAction* Action, TSubclassOf<ANexusCue> CueClass, const FNexusCueParameters& CueParameters)
{
	check(Action);
	AActor* ActorOwner = Cast<AActor>(Action->GetOuter());
	check(ActorOwner);
	if (UNexusActionComponent* ActionComponent = GetActionComponentFromActor(ActorOwner))
	{
		ActionComponent->GetCueComponent()->TriggerCue(CueClass, ActionComponent->GetCurrentPredictionTag(), CueParameters);
	}
}

void UNexusActionComponent::BP_AuthEndCue(TSubclassOf<ANexusCue> CueClass)
{
	GetCueComponent()->AuthEndCue(CueClass);
}

void UNexusActionComponent::SimTriggerCue(TSubclassOf<ANexusCue> CueClass, FNexusPredictionTag PredictionTag, const FNexusCueParameters& CueParameters)
{
	GetCueComponent()->SimTriggerCue(CueClass, PredictionTag, CueParameters);
}

// ------------------------------------------------------------------------------
// Event
// ------------------------------------------------------------------------------
void UNexusActionComponent::HandleEvent(FGameplayTag EventTag, const void* Message, UScriptStruct* MessageType)
{
	GetEventManagerComponent()->HandleEvent(EventTag, Message, MessageType);
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
	return GetEventManagerComponent()->BindEventCallbackDirect(EventTag, MoveTemp(Callbacks), MessageType);
}

void UNexusActionComponent::UnbindEventCallback(FNexusEventCallbackHandle Handle) const
{
	GetEventManagerComponent()->UnbindEventCallback(Handle);
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

UNexusEventManagerComponent* UNexusActionComponent::GetEventManagerComponent() const
{
	return GetCachedComponent(EventManagerComponentCached);
}
