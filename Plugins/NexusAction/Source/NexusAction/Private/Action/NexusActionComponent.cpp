// Fill out your copyright notice in the Description page of Project Settings.


#include "Action/NexusActionComponent.h"
#include "NexusActionInterface.h"
#include "NexusLog.h"
#include "Action/NexusAction.h"
#include "Action/NexusActionListScopeLock.h"
#include "Action/SubComponent/NexusCueComponent.h"
#include "Action/SubComponent/NexusGameplayTagComponent.h"
#include "Action/SubComponent/NexusPropertyComponent.h"
#include "Action/SubComponent/NexusSideEffectComponent.h"
#include "Engine/Canvas.h"
#include "Event/NexusEventManagerComponent.h"
#include "Event/NexusEventMessage.h"
#include "GameFramework/HUD.h"
#include "Net/UnrealNetwork.h"
#include "Prediction/NexusPredictionScope.h"
#include "SideEffect/NexusSideEffect.h"
#include "SideEffect/NexusSideEffectDef.h"

bool bShowActionFailedReason = false;
FAutoConsoleVariableRef ActionSystemShowActionTriggerFailedReasonCmd(
	TEXT("ActionSystem.Debug.ActionTriggerFailedReason"),
	bShowActionFailedReason,
	TEXT("")
);


UNexusActionComponent::UNexusActionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	bWantsInitializeComponent = true;
}

void UNexusActionComponent::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y)
{
	AActor* DebugTarget = HUD->GetCurrentDebugTargetActor();
	if (!DebugTarget)
	{
		return;
	}

	if (UNexusActionComponent* ActionComponent = GetActionComponentFromActor(DebugTarget))
	{
		ActionComponent->InternalOnShowDebugInfo(DebugTarget, HUD, Canvas, DebugDisplayInfo, X, Y);
	}
}

void UNexusActionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UNexusActionComponent, ActionDefs, COND_OwnerOnly);
}

void UNexusActionComponent::InitializeComponent()
{
	Super::InitializeComponent();
	EnsureSubComponent<UNexusCueComponent>();
	EnsureSubComponent<UNexusSideEffectComponent>();
	EnsureSubComponent<UNexusPredictionComponent>();
	EnsureSubComponent<UNexusPropertyComponent>();
	EnsureSubComponent<UNexusGameplayTagComponent>();
	EventManagerComponent = NewObject<UNexusEventManagerComponent>(GetOwner());
	EventManagerComponent->RegisterComponent();
}

void UNexusActionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	TeardownActionComponent();
	Super::EndPlay(EndPlayReason);
}

UNexusActionComponent* UNexusActionComponent::GetActionComponentFromActor(AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}

	if (UNexusActionComponent* ActionComponent = Actor->GetComponentByClass<UNexusActionComponent>())
	{
		return ActionComponent;
	}

	if (INexusActionInterface* ActionInterface = Cast<INexusActionInterface>(Actor))
	{
		return ActionInterface->GetActionComponent();
	}

	return nullptr;
}

void UNexusActionComponent::SetupActionComponent(AActor* InAgentActor)
{
	bSetupCompleted = true;
	InternalSetupActionComponent(InAgentActor);
	OnSetupActionComponent();
	if (OnActionComponentSetupCompletedDelegate.IsBound())
	{
		OnActionComponentSetupCompletedDelegate.Broadcast();
	}
}

void UNexusActionComponent::TeardownActionComponent()
{
	bSetupCompleted = false;
	if (GetOwner()->HasAuthority())
	{
		AuthRemoveAllActions();
		GetPropertyComponent()->AuthRemoveAllProperties();
		GetCueComponent()->RemoveAllLoopingCues();
	}

	if (OnActionComponentTeardownCompletedDelegate.IsBound())
	{
		OnActionComponentTeardownCompletedDelegate.Broadcast();
	}
}

void UNexusActionComponent::CallOrAddSetupCompletedDelegate(FOnNexusActionComponentSetupCompletedSignature::FDelegate&& Delegate)
{
	if (bSetupCompleted)
	{
		Delegate.ExecuteIfBound();
		return;
	}
	OnActionComponentSetupCompletedDelegate.Add(MoveTemp(Delegate));
}

void UNexusActionComponent::RemoveSetupCompletedDelegate(const void* Object)
{
	OnActionComponentSetupCompletedDelegate.RemoveAll(Object);
}

void UNexusActionComponent::AddSetupCompletedDelegate(FOnNexusActionComponentSetupCompletedSignature::FDelegate&& Delegate)
{
	OnActionComponentSetupCompletedDelegate.Add(MoveTemp(Delegate));
}

void UNexusActionComponent::InternalSetupActionComponent(AActor* InAgentActor)
{
	check(InAgentActor);
	TSharedPtr<FNexusAgentInfo> OldAgentInfo = AgentInfo;
	AgentInfo = MakeShared<FNexusAgentInfo>(GetOwner(), InAgentActor);
	for (TObjectPtr<UNexusAgentBoundComponent> SubComponent : SubComponents)
	{
		SubComponent->Setup(AgentInfo);
	}

	if (!ActionDefs.OnActionDefAddedDelegate.IsBound())
	{
		if (OldAgentInfo != AgentInfo && !GetOwner()->HasAuthority() && AgentInfo->IsLocallyControlled())
		{
			for (auto& ActionDef : ActionDefs.Items)
			{
				OnActionDefAdded(ActionDef);
			}
		}
	}
	ActionDefs.OnActionDefAddedDelegate.BindUObject(this, &UNexusActionComponent::OnActionDefAdded);
	ActionDefs.OnActionDefRemovedDelegate.BindUObject(this, &UNexusActionComponent::OnActionDefRemoved);

	if (GetOwner()->HasAuthority())
	{
		AuthRemoveAllActions();
	}
}

void UNexusActionComponent::InternalOnShowDebugInfo(AActor* DebugTarget, AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y)
{
	if (!AgentInfo->AgentActor.IsValid() || DebugTarget != AgentInfo->AgentActor.Get())
	{
		return;
	}


	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	if (HUD->ShouldDisplayDebug(TEXT("ActionSystem")))
	{
		DisplayDebugManager.SetFont(GEngine->GetTinyFont());
		DisplayDebugManager.SetDrawColor(FColor::White);
		FGameplayTagContainer OwnedTags;

		TMap<FGameplayTag, FString> PropertyLogs;

		for (const FNexusSideEffectDef& Item : GetSideEffectComponent()->GetSideEffectDefs().Items)
		{
			if (Item.SideEffectInstance->DurationType == ESideEffectDurationType::Instant)
			{
				continue;
			}

			for (const FNexusPropertyMod& Mod : Item.SideEffectInstance->Modifiers)
			{
				float Value = 0.0f;
				switch (Mod.CalculationType)
				{
				case ENexusPropertyCalculationType::Direct:
					Value = Mod.DirectValue;
					break;
				case ENexusPropertyCalculationType::FromOutside:
					{
						const float* ValuePtr = Item.SideEffectInstance->GetInjectedValues().Find(Mod.InjectedValueTag);
						Value = ValuePtr ? *ValuePtr : 0.0f;
						break;
					}

				case ENexusPropertyCalculationType::PropertyBased:
					{
						UNexusProperty* Property = GetProperty(Mod.PropertyTag);
						Value = Property ? Property->GetDynamicValue() : 0.0f;
						break;
					}
				default:
					continue;
				}
				FString& LogString = PropertyLogs.FindOrAdd(Mod.PropertyTag);
				UEnum* StaticOperatorEnum = StaticEnum<ENexusPropertyOperator>();
				check(StaticOperatorEnum);
				FString EffectNameString = FString::Printf(TEXT("%s (%s) - "), *Item.SideEffectInstance->GetName(), *Item.Handle.ToString());
				FString ModString = FString::Printf(TEXT("%s %.2f "), *StaticOperatorEnum->GetNameStringByValue(static_cast<int64>(Mod.Operator)), Value);
				FString TimeString = FString::Printf(TEXT("[%.2f"), Item.SideEffectInstance->GetElapsedTime());
				FString DurationString = Item.SideEffectInstance->DurationType == ESideEffectDurationType::Infinite
					                         ? TEXT("/INF]")
					                         : FString::Printf(TEXT("/%.2f]"), Item.SideEffectInstance->Duration);
				FString IntervalString = Item.SideEffectInstance->Interval <= 0.0f
					                         ? TEXT("")
					                         : FString::Printf(TEXT(" (Interval: %.2f Applied: %d)"), Item.SideEffectInstance->Interval, Item.SideEffectInstance->GetAppliedCount());
				LogString += EffectNameString + ModString + TimeString + DurationString + IntervalString + TEXT("\n");
			}
		}


		for (UNexusProperty* Property : GetPropertyComponent()->GetProperties())
		{
			DisplayDebugManager.SetDrawColor(FColor::White);
			FString TagString = Property->GetTag().ToString();
			check(TagString.RemoveFromStart(TEXT("Property.")));
			DisplayDebugManager.DrawString(FString::Printf(TEXT("%s: %.2f (정적 값: %.2f)"), *TagString, Property->GetDynamicValue(), Property->GetStaticValue()));
			DisplayDebugManager.SetDrawColor(FColor::Orange);
			if (PropertyLogs.Contains(Property->GetTag()))
			{
				DisplayDebugManager.DrawString(PropertyLogs[Property->GetTag()], 4.0f);
			}
		}

		FString TagString;
		for (const auto& [Tag, Count] : GetGameplayTagComponent()->GetDynamicTagCountMap())
		{
			DisplayDebugManager.SetDrawColor(FColor::White);
			TagString += FString::Printf(TEXT("%s(%d) "), *Tag.ToString(), Count);
		}
		DisplayDebugManager.DrawString(FString::Printf(TEXT("소유 태그: %s"), *TagString));
	}
}


// ------------------------------------------------------------------------------
// Action Add/Remove
// ------------------------------------------------------------------------------
FNexusActionDefHandle UNexusActionComponent::AuthAddAction(const FNexusActionDef& ActionDef)
{
	if (!ensure(GetOwner()->HasAuthority()))
	{
		NX_LOG_SUB_FN(LogNexus, Warning, TEXT("함수는 서버에서만 호출 가능합니다."));
		return {};
	}
	check(ActionDef.ActionClass);

	if (ActionScopeLockCount > 0)
	{
		ActionPendingAdds.Add({ActionDef, false});
		return ActionDef.Handle;
	}
	ACTION_LIST_SCOPE_LOCK();

	ActionDefs.AuthAdd(ActionDef);

	return ActionDef.Handle;
}

void UNexusActionComponent::AuthRemoveAction(const FNexusActionDefHandle& ActionDefHandle)
{
	if (!ensure(GetOwner()->HasAuthority()))
	{
		NX_LOG_SUB_FN(LogNexus, Warning, TEXT("함수는 서버에서만 호출 가능합니다."));
		return;
	}

	FNexusActionDef* ActionDef = FindActionDefByHandle(ActionDefHandle);
	if (!ActionDef)
	{
		return;
	}
	NX_VLOG_SUB(GetOwner(), LogNexusAction, Log, TEXT("액션 [%s] 제거"), *ActionDef->ActionInstance->GetName());

	if (ActionScopeLockCount > 0)
	{
		ActionPendingRemoves.Add(ActionDefHandle);
		return;
	}

	ActionDef->ActionInstance->CallOnEndAction();
	HandleTriggerableActionOnRemoved(*ActionDef);

	ACTION_LIST_SCOPE_LOCK();
	ActionDefs.AuthRemove(ActionDefHandle);
}

void UNexusActionComponent::AuthRemoveAllActions()
{
	if (!ensure(GetOwner()->HasAuthority()))
	{
		NX_LOG_SUB_FN(LogNexus, Warning, TEXT("함수는 서버에서만 호출 가능합니다."));
		return;
	}

	ACTION_LIST_SCOPE_LOCK();
	for (const auto& ActionDef : ActionDefs.Items)
	{
		ActionDef.ActionInstance->CallOnEndAction();
		HandleTriggerableActionOnRemoved(ActionDef);
		NX_VLOG_SUB(GetOwner(), LogNexusAction, Log, TEXT("액션 [%s] 제거"), *ActionDef.ActionInstance->GetName());
	}
	ActionDefs.AuthRemoveAll();
}

void UNexusActionComponent::IncreaseActionListLock()
{
	ActionScopeLockCount++;
}

void UNexusActionComponent::DecreaseActionListLock()
{
	ActionScopeLockCount--;
	if (ActionScopeLockCount == 0 && (ActionPendingAdds.IsEmpty() || ActionPendingRemoves.IsEmpty()))
	{
		TArray<FNexusPendingAddActionInfo> PendingAdds = MoveTemp(ActionPendingAdds);
		TArray<FNexusActionDefHandle> PendingRemoves = MoveTemp(ActionPendingRemoves);

		for (const auto& PendingAddInfo : PendingAdds)
		{
			if (GetOwner()->HasAuthority())
			{
				AuthAddAction(PendingAddInfo.ActionDef);
				if (PendingAddInfo.bIsPendingTrigger)
				{
					TryTriggerAction(PendingAddInfo.ActionDef.Handle, PendingAddInfo.PendingEventMessage);
				}
			}
		}

		for (const auto& ActionDef : PendingRemoves)
		{
			if (GetOwner()->HasAuthority())
			{
				AuthRemoveAction(ActionDef);
			}
		}
	}
}

void UNexusActionComponent::OnActionDefAdded(FNexusActionDef& ActionDef)
{
	ActionDef.ActionInstance = NewObject<UNexusAction>(GetOwner(), ActionDef.ActionClass);
	ActionDef.ActionInstance->OnActionEndedDelegate.AddUObject(this, &UNexusActionComponent::OnActionEnded);
	ActionDef.ActionInstance->InitializeAction(ActionDef.Handle, AgentInfo);
	ActionDef.ActionInstance->CallOnActionAdded();

	NX_VLOG_SUB(GetOwner(), LogNexusAction, Log, TEXT("액션 [%s] 추가"), *ActionDef.ActionInstance->GetName());
	HandleTriggerableActionOnAdded(ActionDef);
}

void UNexusActionComponent::OnActionDefRemoved(FNexusActionDef& ActionDef)
{
	ActionDef.ActionInstance->CallOnEndAction();
	ActionDef.ActionInstance->CallOnActionRemoved();
	HandleTriggerableActionOnRemoved(ActionDef);
}

void UNexusActionComponent::HandleTriggerableActionOnAdded(const FNexusActionDef& NewActionDef)
{
	if (!HasActionTriggerAuthority(NewActionDef.ActionInstance))
	{
		return;
	}

	BindActionTriggerEvent(NewActionDef);
	if (NewActionDef.ActionInstance->ShouldTriggerOnAdded())
	{
		TryTriggerAction(NewActionDef.Handle, FNexusEventMessage());
	}

	ClientPendingActionTriggerRequests.RemoveAll([NewActionDefHandle = NewActionDef.Handle, this](const FNexusPendingActionTriggerRequest& Request)
	{
		if (Request.ActionDefHandle == NewActionDefHandle)
		{
			TryTriggerAction(Request.ActionDefHandle, Request.EventMessage);
			return true;
		}
		return false;
	});
}

void UNexusActionComponent::HandleTriggerableActionOnRemoved(const FNexusActionDef& ActionDef)
{
	if (HasActionTriggerAuthority(ActionDef.ActionInstance))
	{
		UnbindActionTriggerEvent(ActionDef);
	}
}

void UNexusActionComponent::BindActionTriggerEvent(const FNexusActionDef& NewActionDef)
{
	UNexusAction* Action = NewActionDef.ActionInstance;

	FGameplayTagContainer ActionTriggerEventTags = Action->GetActionTriggerEventTags();
	for (FGameplayTag Tag : ActionTriggerEventTags)
	{
		FNexusEventCallbackHandle EventCallbackHandle = EventManagerComponent->BindEventCallback<FNexusEventMessage>(Tag, this, &ThisClass::OnActionEventTriggered, NewActionDef.Handle);
		BoundedActionEventHandles.FindOrAdd(NewActionDef.Handle).Add(EventCallbackHandle);
	}
}

void UNexusActionComponent::UnbindActionTriggerEvent(const FNexusActionDef& ActionDef)
{
	if (TArray<FNexusEventCallbackHandle>* EventCallbackHandles = BoundedActionEventHandles.Find(ActionDef.Handle))
	{
		for (FNexusEventCallbackHandle EventCallbackHandle : *EventCallbackHandles)
		{
			EventManagerComponent->UnbindEventCallback(EventCallbackHandle);
		}
		BoundedActionEventHandles.Remove(ActionDef.Handle);
	}
}


// ------------------------------------------------------------------------------
// Action Trigger
// ------------------------------------------------------------------------------
bool UNexusActionComponent::HasActionTriggerAuthority(UNexusAction* Action) const
{
	check(Action);
	ENexusActionNetMethod ActionNetMethod = Action->GetActionNetMethod();
	if (ActionNetMethod == ENexusActionNetMethod::LocalOnly || ActionNetMethod == ENexusActionNetMethod::LocalPredicted)
	{
		return AgentInfo->IsLocallyControlled() || Action->IsRemoteTriggerable();
	}

	if (ActionNetMethod == ENexusActionNetMethod::ServerOnly || ActionNetMethod == ENexusActionNetMethod::ServerAuthoritative)
	{
		return GetOwner()->HasAuthority() || Action->IsRemoteTriggerable();
	}

	return false;
}

bool UNexusActionComponent::CanTriggerAction(FNexusActionDef* ActionDef, const FNexusEventMessage& EventMessage)
{
	ActionDef->ActionInstance->SetActionCurrentEventMessage(EventMessage);
	return InternalCanTriggerAction(*ActionDef);
}

void UNexusActionComponent::TryTriggerAction(FNexusActionDefHandle ActionDefHandle, const FNexusEventMessage& EventMessage)
{
	check(ActionDefHandle.IsValid());


	FNexusActionDef* ActionDef = FindActionDefByHandle(ActionDefHandle);

	// 1. PendingAdd (ActionDef == nullptr)
	// 2. 서버로부터 아직 Replicate안됨 (ActionDef == nullptr)
	// 3. 아직 ActionComponent가 Setup안됨 (ActionDef && !ActionDef->ActionInstance)

	if (!ActionDef) // 액션 데피니션이 추가에 대한 보류 중일 경우 액션 데피니션이 없을 수 있음 
	{
		FNexusPendingAddActionInfo* FoundPendingInfo = ActionPendingAdds.FindByPredicate([ActionDefHandle](const FNexusPendingAddActionInfo& PendingActionInfo)
		{
			return PendingActionInfo.ActionDef.Handle == ActionDefHandle;
		});
		if (ensure(FoundPendingInfo)) // TODO: 서버로부터 아직 Replicate 안됐을 때 처리
		{
			FoundPendingInfo->bIsPendingTrigger = true;
		}
		return;
	}

	if (!bSetupCompleted)
	{
		ClientPendingActionTriggerRequests.Add({ActionDefHandle, EventMessage});
		return;
	}

	check(AgentInfo->OwnerActor.IsValid());
	check(ActionDef->ActionInstance);

	ensureMsgf(ActionDef, TEXT("AddAction을 통해 먼저 해당 Action을 부여해야 합니다"));
	ensureMsgf(AgentInfo->AgentActor->GetLocalRole() != ROLE_SimulatedProxy,
	           TEXT("Agent Actor: [%s]가 SimulatedProxy인 경우 액션 [%s]을(를) 실행할 수 없습니다"), *AgentInfo->AgentActor->GetName(),
	           *ActionDef->ActionInstance->GetName());

	if (!CanTriggerAction(ActionDef, EventMessage))
	{
		return;
	}


	const bool bIsOwnerActorAuthoritative = AgentInfo->IsOwnerActorAuthoritative();
	FNexusPredictionTag PredictionTag;
	PredictionTag.GenerateNewHandle(bIsOwnerActorAuthoritative);
	FNexusPredictionScope PredictionScope(*GetPredictionComponent(), PredictionTag);
	ActionDef->ActionInstance->SetPrimaryPredictionTag(GetPredictionComponent()->GetCurrentPredictionTag());


	if (AgentInfo->OwnerActor->GetNetMode() == NM_Standalone)
	{
		LocalTriggerAction(ActionDef);
		return;
	}

	const bool bIsLocallyControlled = AgentInfo->IsLocallyControlled();

	const ENexusActionNetMethod ActionNetMethod = ActionDef->ActionInstance->GetActionNetMethod();
	const bool bIsRemoteTriggerable = ActionDef->ActionInstance->IsRemoteTriggerable();


	if (bIsOwnerActorAuthoritative)
	{
		if (bIsLocallyControlled || ActionNetMethod == ENexusActionNetMethod::ServerOnly)
		{
			LocalTriggerAction(ActionDef);
			return;
		}

		if (ActionNetMethod == ENexusActionNetMethod::ServerAuthoritative)
		{
			LocalTriggerAction(ActionDef);
			ClientTriggerAction(ActionDefHandle, EventMessage, GetPredictionComponent()->GetCurrentPredictionTag());
			return;
		}

		if (bIsRemoteTriggerable)
		{
			ClientRemoteRequestTryTriggerAction(ActionDefHandle, EventMessage);
			return;
		}

		checkNoEntry(); // 해당 호스트에서는 Action을(를) 실행할 수 없습니다.
		return;
	}

	if (bIsLocallyControlled) // Autonomous Proxy
	{
		if (ActionNetMethod == ENexusActionNetMethod::LocalOnly)
		{
			LocalTriggerAction(ActionDef);
			return;
		}

		if (ActionNetMethod == ENexusActionNetMethod::LocalPredicted)
		{
			LocalTriggerAction(ActionDef);
			ServerTryTriggerAction(ActionDefHandle, EventMessage, GetPredictionComponent()->GetCurrentPredictionTag());
			return;
		}

		if (bIsRemoteTriggerable)
		{
			ServerRemoteRequestTryTriggerAction(ActionDefHandle, EventMessage);
			return;
		}

		checkNoEntry(); // 해당 호스트에서는 Action을(를) 실행할 수 없습니다.
	}
}

bool UNexusActionComponent::InternalCanTriggerAction(const FNexusActionDef& ActionDef) const
{
	bool bIsNotTriggeringOrRetriggerable = ActionDef.ActionInstance->IsRetriggerable() || !ActionDef.ActionInstance->IsTriggering();
	bool bMetTriggerCondition = ActionDef.ActionInstance->CallOnCanTriggerAction();
	FGameplayTagContainer OwnedTags;
	GetOwnedGameplayTags(OwnedTags);
	bool bHasRequiredTags = OwnedTags.HasAllExact(ActionDef.ActionInstance->GetShouldHaveTags());
	bool bDontHaveForbiddenTags = !OwnedTags.HasAnyExact(ActionDef.ActionInstance->GetShouldNotHaveTags());

	if (!bShowActionFailedReason)
	{
		return bIsNotTriggeringOrRetriggerable && bMetTriggerCondition && bHasRequiredTags && bDontHaveForbiddenTags;
	}

	if (!bIsNotTriggeringOrRetriggerable)
	{
		NX_LOG_SUB(LogNexusAction, Log, TEXT("액션 [%s] 거부 (실행 중)"), *ActionDef.ActionInstance->GetName());
		return false;
	}

	if (!bMetTriggerCondition)
	{
		NX_LOG_SUB(LogNexusAction, Log, TEXT("액션 [%s] 거부 (조건 미충족)"), *ActionDef.ActionInstance->GetName());
		return false;
	}

	if (!bHasRequiredTags)
	{
		NX_LOG_SUB(LogNexusAction, Log, TEXT("액션 [%s] 거부 (필수 태그 미보유)"), *ActionDef.ActionInstance->GetName());
		const FGameplayTagContainer& ShouldHaveTags = ActionDef.ActionInstance->GetShouldHaveTags();
		FGameplayTagContainer NotOwnedTags;
		for (FGameplayTag Tag : ShouldHaveTags)
		{
			if (!OwnedTags.HasTag(Tag))
			{
				NotOwnedTags.AddTag(Tag);
			}
		}
		NX_LOG_SUB(LogNexusAction, Log, TEXT("미보유 태그: %s"), *NotOwnedTags.ToStringSimple(true));
		return false;
	}

	if (!bDontHaveForbiddenTags)
	{
		NX_LOG_SUB(LogNexusAction, Log, TEXT("액션 [%s] 거부 (금지 태그 보유)"), *ActionDef.ActionInstance->GetName());
		const FGameplayTagContainer& ShouldNotHaveTags = ActionDef.ActionInstance->GetShouldNotHaveTags();
		FGameplayTagContainer OwnedForbiddenTags;
		for (FGameplayTag Tag : ShouldNotHaveTags)
		{
			if (OwnedTags.HasTag(Tag))
			{
				OwnedForbiddenTags.AddTag(Tag);
			}
		}
		NX_LOG_SUB(LogNexusAction, Log, TEXT("보유 금지 태그: %s"), *OwnedForbiddenTags.ToStringSimple(true));
		return false;
	}

	return true;
}

void UNexusActionComponent::LocalTriggerAction(FNexusActionDef* ActionDef)
{
	NX_LOG_SUB(LogNexusAction, Log, TEXT( "액션 [%s] 실행" ), *ActionDef->ActionInstance->GetName());
	check(ActionDef->ActionInstance);
	ActionDef->ActionInstance->CallOnTriggerAction();
	if (IsOwnerActorAuthoritative())
	{
		LocalOnTriggerActionConfirmed(ActionDef->Handle, ActionDef->ActionInstance->GetPrimaryPredictionTag());
	}

	if (!ActionDef->ActionInstance->GetActionOwnedTags().IsEmpty())
	{
		FNexusSideEffectDef TagSideEffectDef = MakeSideEffectDef(ActionDef->ActionInstance, UNexusSideEffect::StaticClass());
		TagSideEffectDef.SideEffectInstance->DurationType = ESideEffectDurationType::Infinite;
		FNexusGameplayTagMod TagMod;
		TagMod.TagsToGrant.AppendTags(ActionDef->ActionInstance->GetActionOwnedTags());
		TagSideEffectDef.SideEffectInstance->TagModifiers.Add(TagMod);
		TriggerSideEffectByDef(TagSideEffectDef, ActionDef->ActionInstance);
		if (IsOwnerActorAuthoritative())
		{
			TagSideEffectMap.Add({ActionDef->Handle, TagSideEffectDef.Handle});
		}
	}
}

void UNexusActionComponent::ServerTryTriggerAction_Implementation(FNexusActionDefHandle ActionDefHandle, const FNexusEventMessageReplicated& EventMessageReplicated, FNexusPredictionTag PredictionTag)
{
	if (!ActionDefHandle.IsValid())
	{
		unimplemented(); // FAIL DESYNC
	}
	FNexusActionDef* ActionDef = FindActionDefByHandle(ActionDefHandle);
	if (!ActionDef)
	{
		unimplemented(); // FAIL DESYNC
	}

	if (!CanTriggerAction(ActionDef, EventMessageReplicated.ToEventMessage()))
	{
		ClientTriggerActionRequestFailed(ActionDefHandle, PredictionTag);
		return;
	}

	ClientTriggerActionRequestSucceeded(ActionDefHandle, PredictionTag);
	FNexusPredictionScope PredictionScope(*GetPredictionComponent(), PredictionTag);
	ActionDef->ActionInstance->SetPrimaryPredictionTag(GetPredictionComponent()->GetCurrentPredictionTag());
	LocalTriggerAction(ActionDef);
}

void UNexusActionComponent::ClientTriggerAction_Implementation(FNexusActionDefHandle ActionDefHandle, const FNexusEventMessageReplicated& EventMessageReplicated, FNexusPredictionTag PredictionTag)
{
	check(ActionDefHandle.IsValid());
	FNexusActionDef* ActionDef = FindActionDefByHandle(ActionDefHandle);
	check(ActionDef);
	ActionDef->ActionInstance->SetActionCurrentEventMessage(EventMessageReplicated.ToEventMessage());
	FNexusPredictionScope PredictionScope(*GetPredictionComponent(), PredictionTag);
	ActionDef->ActionInstance->SetPrimaryPredictionTag(GetPredictionComponent()->GetCurrentPredictionTag());
	LocalTriggerAction(ActionDef);
}

void UNexusActionComponent::ClientTriggerActionRequestSucceeded_Implementation(FNexusActionDefHandle ActionDefHandle, FNexusPredictionTag PredictionTag)
{
	LocalOnTriggerActionConfirmed(ActionDefHandle, PredictionTag);
}

void UNexusActionComponent::ClientTriggerActionRequestFailed_Implementation(FNexusActionDefHandle ActionDefHandle, FNexusPredictionTag PredictionTag)
{
	FNexusActionDef* ActionDef = FindActionDefByHandle(ActionDefHandle);
	check(ActionDef);
	ActionDef->ActionInstance->CallOnEndAction();
	FNexusPredictionEvents::BroadcastOnPredictionFailed(PredictionTag);
}

void UNexusActionComponent::ServerRemoteRequestTryTriggerAction_Implementation(FNexusActionDefHandle ActionDefHandle, const FNexusEventMessage& EventMessage)
{
	TryTriggerAction(ActionDefHandle, EventMessage);
}

void UNexusActionComponent::ClientRemoteRequestTryTriggerAction_Implementation(FNexusActionDefHandle ActionDefHandle, const FNexusEventMessage& EventMessage)
{
	TryTriggerAction(ActionDefHandle, EventMessage);
}

void UNexusActionComponent::LocalOnTriggerActionConfirmed(FNexusActionDefHandle ActionDefHandle, FNexusPredictionTag PredictionTag)
{
	NX_VLOG_SUB(GetOwner(), LogNexusAction, Log, TEXT("액션 [%s] 승인 완료"), *ActionDefHandle.ToString());
	FNexusActionDef* ActionDef = FindActionDefByHandle(ActionDefHandle);
	check(ActionDef);
	const FGameplayTagContainer& CancelTags = ActionDef->ActionInstance->GetActionCancelTags();
	if (!CancelTags.IsEmpty())
	{
		TArray<FNexusActionDef> TriggeringActionDefs = ActionDefs.GetAllTriggeringActionDefs();
		for (FNexusActionDef& TriggeringActionDef : TriggeringActionDefs)
		{
			if (TriggeringActionDef.ActionInstance->GetActionOwnedTags().HasAnyExact(CancelTags))
			{
				TriggeringActionDef.ActionInstance->CallOnEndAction();
			}
		}
	}
}

void UNexusActionComponent::OnActionEventTriggered(FGameplayTag GameplayTag, const FNexusEventMessage& EventMessage, FNexusActionDefHandle ActionDefHandle)
{
	TryTriggerAction(ActionDefHandle, EventMessage);
}

void UNexusActionComponent::OnActionEnded(FNexusActionDefHandle ActionDefHandle, UNexusAction* Action)
{
	if (IsOwnerActorAuthoritative())
	{
		if (FNexusSideEffectDefHandle* TagSideEffectHandle = TagSideEffectMap.Find(ActionDefHandle))
		{
			GetSideEffectComponent()->RemoveSideEffect(*TagSideEffectHandle);
			TagSideEffectMap.Remove(ActionDefHandle);
		}
	}


	NX_VLOG_SUB(GetOwner(), LogNexusAction, Log, TEXT("액션 [%s] 종료"), *Action->GetName());
}

FNexusActionDefHandle UNexusActionComponent::FindActionDefHandle(TSubclassOf<UNexusAction> ActionClass, UObject* SourceObject) const
{
	return ActionDefs.FindActionDefHandle(ActionClass, SourceObject);
}

FNexusActionDef* UNexusActionComponent::FindActionDefByHandle(FNexusActionDefHandle ActionDefHandle)
{
	check(ActionDefHandle.IsValid());
	if (FNexusActionDef* ActionDefPtr = ActionDefs.FindActionDefByHandle(ActionDefHandle))
	{
		return ActionDefPtr;
	}

	return nullptr;
}

FNexusPredictionTag UNexusActionComponent::GetCurrentPredictionTag() const
{
	return GetPredictionComponent()->GetCurrentPredictionTag();
}
