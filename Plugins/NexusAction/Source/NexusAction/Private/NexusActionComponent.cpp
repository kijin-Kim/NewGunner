// Fill out your copyright notice in the Description page of Project Settings.


#include "NexusActionComponent.h"
#include "Action/NexusAction.h"
#include "NexusActionInterface.h"
#include "Cue/NexusCue.h"
#include "NexusPredictionScope.h"
#include "SideEffect/NexusSideEffect.h"
#include "SideEffect/NexusSideEffectDef.h"
#include "Engine/ActorChannel.h"
#include "Engine/Canvas.h"
#include "GameFramework/HUD.h"


#include "Event/NexusEventMessage.h"
#include "NexusLog.h"
#include "Event/NexusEventManagerComponent.h"
#include "Net/UnrealNetwork.h"

bool bShowActionFailedReason = false;
FAutoConsoleVariableRef ActionSystemShowActionTriggerFailedReasonCmd(
	TEXT("ActionSystem.Debug.ActionTriggerFailedReason"),
	bShowActionFailedReason,
	TEXT("")
);


UNexusActionComponent::UNexusActionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	AgentInfo = MakeShared<FNexusAgentInfo>();
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		AHUD::OnShowDebugInfo.AddStatic(&ThisClass::OnShowDebugInfo);
	}
}

void UNexusActionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	FNexusPredictionEvents::Clear();
}

void UNexusActionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UNexusActionComponent, ActionDefinitions, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UNexusActionComponent, NetPredictionHandles, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UNexusActionComponent, SideEffectDefinitions, COND_OwnerOnly);
	DOREPLIFETIME(UNexusActionComponent, Properties);
}


bool UNexusActionComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	for (UNexusProperty* Property : Properties)
	{
		bWroteSomething |= Channel->ReplicateSubobject(Property, *Bunch, *RepFlags);
	}
	return bWroteSomething;
}

void UNexusActionComponent::InitActionComponent(AActor* InAgentActor)
{
	check(InAgentActor);
	FNexusAgentInfo OldAgentInfo = *AgentInfo;
	AgentInfo->Init(GetOwner(), InAgentActor);
	SideEffectDefinitions.Init(GetOwner());
	ActionDefinitions.OnActionDefinitionAddedDelegate.BindUObject(this, &UNexusActionComponent::OnActionDefinitionAdded);
	ActionDefinitions.OnActionDefinitionRemovedDelegate.BindUObject(this, &UNexusActionComponent::OnActionDefinitionRemoved);


	if (GetOwner()->HasAuthority())
	{
		AuthRemoveAllActions();
	}

	if (OldAgentInfo != *AgentInfo && !GetOwner()->HasAuthority() && AgentInfo->IsLocallyControlled())
	{
		for (auto& ActionDefinition : ActionDefinitions.Items)
		{
			OnActionDefinitionAdded(ActionDefinition);
		}
	}
}

void UNexusActionComponent::ReleaseActionComponent()
{
	if (GetOwner()->HasAuthority())
	{
		AuthRemoveAllActions();
		AuthRemoveAllProperties();
		OwnedTags.Reset();
	}
}

void UNexusActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	for (UNexusProperty* Property : Properties)
	{
		if (Property)
		{
			Property->Tick();
		}
	}
	SideEffectDefinitions.Tick(DeltaTime);
}

FNexusActionDefHandle UNexusActionComponent::AuthAddAction(const FNexusActionDef& ActionDefinition)
{
	check(GetOwner()->HasAuthority());

	if (ActionScopeLockCount > 0)
	{
		ActionPendingAdds.Add(ActionDefinition);
		return ActionDefinition.Handle;
	}

	ACTION_LIST_SCOPE_LOCK();

	FNexusActionDef NewActionDefinition = ActionDefinition;
	NewActionDefinition.ActionInstance = NewAction<UNexusAction>(NewActionDefinition.ActionClass, NewActionDefinition.Handle, AgentInfo);
	ActionDefinitions.AuthAdd(NewActionDefinition);
	HandleTriggerableActionOnAdded(NewActionDefinition);

	return NewActionDefinition.Handle;
}

void UNexusActionComponent::AuthRemoveAction(const FNexusActionDefHandle& ActionDefinitionHandle)
{
	check(GetOwner()->HasAuthority());


	FNexusActionDef* ActionDefinition = FindActionDefinitionByHandle(ActionDefinitionHandle);
	if (!ActionDefinition)
	{
		return;
	}
	NX_LOG_SUB(LogNexusAction, Verbose, TEXT("Action [%s] 제거"), *ActionDefinition->ActionInstance->GetName());

	if (ActionScopeLockCount > 0)
	{
		ActionPendingRemoves.Add(ActionDefinitionHandle);
		return;
	}

	ActionDefinition->ActionInstance->EndAction();
	HandleTriggerableActionOnRemoved(*ActionDefinition);

	ACTION_LIST_SCOPE_LOCK();
	ActionDefinitions.AuthRemove(ActionDefinitionHandle);
}

void UNexusActionComponent::AuthRemoveAllActions()
{
	check(GetOwner()->HasAuthority());
	ACTION_LIST_SCOPE_LOCK();
	for (const auto& ActionDefinition : ActionDefinitions.Items)
	{
		ActionDefinition.ActionInstance->EndAction();
		HandleTriggerableActionOnRemoved(ActionDefinition);
		NX_LOG_SUB(LogNexusAction, Verbose, TEXT("Action [%s] 제거"), *ActionDefinition.ActionInstance->GetName());
	}
	ActionDefinitions.AuthRemoveAll();
}

void UNexusActionComponent::TryTriggerAction(FNexusActionDefHandle ActionDefinitionHandle, const FNexusEventMessage& EventMessage)
{
	check(ActionDefinitionHandle.IsValid());
	FNexusActionDef* ActionDefinition = FindActionDefinitionByHandle(ActionDefinitionHandle);

	check(AgentInfo->OwnerActor.IsValid());
	check(ActionDefinition->ActionInstance);

	ensureMsgf(ActionDefinition, TEXT("AddAction을 통해 먼저 해당 Action을 부여해야 합니다"));
	ensureMsgf(AgentInfo->AgentActor->GetLocalRole() != ROLE_SimulatedProxy,
	           TEXT("Agent Actor: [%s]가 SimulatedProxy인 경우 Action [%s]을(를) 실행할 수 없습니다"), *AgentInfo->AgentActor->GetName(),
	           *ActionDefinition->ActionInstance->GetName());

	ActionDefinition->ActionInstance->SetActionCurrentEventMessage(EventMessage);
	if (!CanTriggerAction(*ActionDefinition))
	{
		return;
	}


	if (AgentInfo->OwnerActor->GetNetMode() == NM_Standalone)
	{
		LocalTriggerAction(ActionDefinition);
		return;
	}

	const bool bIsLocallyControlled = AgentInfo->IsLocallyControlled();
	const bool bIsOwnerActorAuthoritative = GetOwner()->HasAuthority();
	const ENexusActionNetMethod ActionNetMethod = ActionDefinition->ActionInstance->GetActionNetMethod();
	const bool bIsRemoteTriggerable = ActionDefinition->ActionInstance->IsRemoteTriggerable();


	if (bIsOwnerActorAuthoritative)
	{
		if (bIsLocallyControlled || ActionNetMethod == ENexusActionNetMethod::ServerOnly)
		{
			LocalTriggerAction(ActionDefinition);
			return;
		}

		if (ActionNetMethod == ENexusActionNetMethod::ServerAuthoritative)
		{
			LocalTriggerAction(ActionDefinition);
			ClientTriggerAction(ActionDefinitionHandle, EventMessage);
			return;
		}

		if (bIsRemoteTriggerable)
		{
			ClientRemoteRequestTryTriggerAction(ActionDefinitionHandle, EventMessage);
			return;
		}

		checkNoEntry(); // 해당 호스트에서는 Action을(를) 실행할 수 없습니다.
		return;
	}

	if (bIsLocallyControlled) // Autonomous Proxy
	{
		if (ActionNetMethod == ENexusActionNetMethod::LocalOnly)
		{
			LocalTriggerAction(ActionDefinition);
			return;
		}

		if (ActionNetMethod == ENexusActionNetMethod::LocalPredicted)
		{
			CurrentPredictionTag.GenerateNewHandle();
			LocalTriggerAction(ActionDefinition, CurrentPredictionTag);
			ServerTryTriggerAction(ActionDefinitionHandle, EventMessage, CurrentPredictionTag);
			return;
		}

		if (bIsRemoteTriggerable)
		{
			ServerRemoteRequestTryTriggerAction(ActionDefinitionHandle, EventMessage);
			return;
		}

		checkNoEntry(); // 해당 호스트에서는 Action을(를) 실행할 수 없습니다.
	}
}

void UNexusActionComponent::ServerSendNetSyncPoint_Implementation(FNexusActionDefHandle Handle, FNexusPredictionTag InitPredictionHandle, FNexusPredictionTag NewPredictionHandle)
{
	int32 Index = NetSyncPointDelegates.Find(FNetSyncPointDelegate::SyncPointDelegateKeyType(Handle, InitPredictionHandle));
	if (Index == INDEX_NONE)
	{
		NetSyncPointDelegates.Add(FNetSyncPointDelegate(FNetSyncPointDelegate::SyncPointDelegateKeyType(Handle, InitPredictionHandle), NewPredictionHandle));
		return;
	}

	if (NetSyncPointDelegates[Index].OnSyncDelegate.IsBound())
	{
		FNexusPredictionScope ScopedNetPrediction(*this, GetOwner()->HasAuthority(), NewPredictionHandle);
		NetSyncPointDelegates[Index].OnSyncDelegate.Broadcast();
	}
	NetSyncPointDelegates.RemoveAt(Index);
}


void UNexusActionComponent::CallOrAddSNetyncPointDelegate(FNexusActionDefHandle Handle, FNexusPredictionTag InitPredictionHandle, FSimpleMulticastDelegate::FDelegate&& Delegate)
{
	int32 Index = NetSyncPointDelegates.Find(FNetSyncPointDelegate::SyncPointDelegateKeyType(Handle, InitPredictionHandle));
	if (Index == INDEX_NONE)
	{
		NetSyncPointDelegates.Add(FNetSyncPointDelegate(FNetSyncPointDelegate::SyncPointDelegateKeyType(Handle, InitPredictionHandle), MoveTemp(Delegate)));
		return;
	}

	FNexusPredictionScope ScopedNetPrediction(*this, GetOwner()->HasAuthority(), NetSyncPointDelegates[Index].NewPredictionHandle);
	Delegate.ExecuteIfBound();
	NetSyncPointDelegates.RemoveAt(Index);
}

void UNexusActionComponent::ReplicatedNetPredictionHandle(const FNexusPredictionTag& PredictionHandle)
{
	NetPredictionHandles.ReplicatedNetPredictionHandle(PredictionHandle);
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
		TArray<FNexusActionDef> PendingAdds = MoveTemp(ActionPendingAdds);
		TArray<FNexusActionDefHandle> PendingRemoves = MoveTemp(ActionPendingRemoves);

		for (const auto& ActionDefinition : PendingAdds)
		{
			if (GetOwner()->HasAuthority())
			{
				AuthAddAction(ActionDefinition);
			}
		}

		for (const auto& ActionDefinition : PendingRemoves)
		{
			if (GetOwner()->HasAuthority())
			{
				AuthRemoveAction(ActionDefinition);
			}
		}
	}
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

FNexusSideEffectDef UNexusActionComponent::MakeSideEffectDefinition(UNexusAction* Action, TSubclassOf<UNexusSideEffect> SideEffectClass)
{
	check(Action);
	FNexusSideEffectDef NewSideEffectDefinition{SideEffectClass};
	NewSideEffectDefinition.SideEffectInstance = NewObject<UNexusSideEffect>(Action->GetOwnerActor(), SideEffectClass);
	return NewSideEffectDefinition;
}

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

void UNexusActionComponent::TriggerSideEffect(TSubclassOf<UNexusSideEffect> SideEffectClass, UNexusAction* Action)
{
	check(Action);
	FNexusSideEffectDef NewSideEffectDefinition = MakeSideEffectDefinition(Action, SideEffectClass);
	TriggerSideEffectByDefinition(NewSideEffectDefinition, Action);
}

void UNexusActionComponent::BP_TriggerSideEffectToActorByDefinition(UNexusAction* Action, AActor* SideEffectTarget, const FNexusSideEffectDef& SideEffectDefinition)
{
	check(Action);
	if (!SideEffectTarget)
	{
		return;
	}

	if (UNexusActionComponent* ActionComponent = GetActionComponentFromActor(SideEffectTarget))
	{
		ActionComponent->TriggerSideEffectByDefinition(SideEffectDefinition, Action);
	}
}

void UNexusActionComponent::TriggerSideEffectByDefinition(const FNexusSideEffectDef& NewSideEffectDefinition, UNexusAction* Action)
{
	check(Action);
	check(!CurrentPredictionTag.IsExpired());
	SideEffectDefinitions.Add(NewSideEffectDefinition, CurrentPredictionTag);

	if (!GetOwner()->HasAuthority() && Action->GetActionNetMethod() == ENexusActionNetMethod::LocalPredicted)
	{
		FNexusPredictionEvents::FPredictionEvent& PredictionEvent = FNexusPredictionEvents::GetPredictionEvent(CurrentPredictionTag);
		PredictionEvent.OnPredictionEnded.AddLambda([this, SideEffectDefinitionHandle = NewSideEffectDefinition.Handle]()
		{
			NX_LOG_SUB(LogNexusSideEffect, Verbose, TEXT("SideEffect [%s] 삭제 (예측 종료)"), *SideEffectDefinitionHandle.ToString());
			SideEffectDefinitions.Remove(SideEffectDefinitionHandle);
		});

		PredictionEvent.OnPredictionFailed.AddLambda([this, SideEffectDefinitionHandle = NewSideEffectDefinition.Handle]()
		{
			NX_LOG_SUB(LogNexusSideEffect, Error, TEXT("SideEffect [%s] 삭제 (예측 실패)"), *SideEffectDefinitionHandle.ToString());
			SideEffectDefinitions.Remove(SideEffectDefinitionHandle);
		});
	}
}

void UNexusActionComponent::BP_Signal(UNexusAction* Action, TSubclassOf<UNexusCue> SignClass, FNexusRepDataHandle TargetData)
{
	check(Action);
	AActor* ActorOwner = Cast<AActor>(Action->GetOuter());
	check(ActorOwner);
	if (UNexusActionComponent* ActionComponent = GetActionComponentFromActor(ActorOwner))
	{
		ActionComponent->Signal(Action, SignClass, TargetData);
	}
}

void UNexusActionComponent::Signal(UNexusAction* Action, TSubclassOf<UNexusCue> SignClass, FNexusRepDataHandle TargetData)

{
	if (!SignClass || !Action)
	{
		return;
	}

	if (GetOwner()->HasAuthority())
	{
		NetMulticastSignal(SignClass, CurrentPredictionTag, TargetData);
		return;
	}

	if (Action->GetActionNetMethod() != ENexusActionNetMethod::ServerAuthoritative)
	{
		InternalSignal(SignClass, TargetData);
	}
}

void UNexusActionComponent::NetMulticastSignal_Implementation(TSubclassOf<UNexusCue> SignClass, FNexusPredictionTag PredictionHandle, FNexusRepDataHandle TargetData)
{
	if (!AgentInfo->IsLocallyControlled() || GetOwner()->HasAuthority() || (AgentInfo->IsLocallyControlled() && !PredictionHandle.IsValid()))
	{
		InternalSignal(SignClass, TargetData);
	}
}


void UNexusActionComponent::InternalSignal(TSubclassOf<UNexusCue> SignClass, FNexusRepDataHandle TargetData)
{
	if (!SignClass)
	{
		return;
	}

	UNexusCue* Sign = SignClass.GetDefaultObject();
	NX_LOG_SUB(LogNexusSideEffect, Verbose, TEXT("Sign [%s] 실행"), *Sign->GetName());
	check(Sign);

	Sign->SetSignalTargetData(TargetData);
	Sign->OnSignaled();
	Sign->SetSignalTargetData(FNexusRepDataHandle());
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
		DisplayDebugManager.SetDrawColor(FColor::Orange);
		for (FGameplayTag Tag : OwnedTags)
		{
			DisplayDebugManager.DrawString(FString::Printf(TEXT("OwnedTags: %s"), *Tag.ToString()));
		}


		for (UNexusProperty* Property : Properties)
		{
			DisplayDebugManager.DrawString(FString::Printf(TEXT("%s: StaticValue: %f, DynamicValue: %f"), *Property->GetTag().ToString(), Property->GetStaticValue(), Property->GetDynamicValue()));
		}


		for (const FNexusSideEffectDef& Item : SideEffectDefinitions.Items)
		{
			DisplayDebugManager.DrawString(FString::Printf(TEXT("SideEffect: %s"), *Item.SideEffectClass->GetName()));
		}
	}
}

void UNexusActionComponent::OnActionDefinitionAdded(FNexusActionDef& ActionDefinition)
{
	ActionDefinition.ActionInstance = NewAction<UNexusAction>(ActionDefinition.ActionClass, ActionDefinition.Handle, AgentInfo);
	HandleTriggerableActionOnAdded(ActionDefinition);
}

void UNexusActionComponent::OnActionDefinitionRemoved(FNexusActionDef& ActionDefinition)
{
	ActionDefinition.ActionInstance->EndAction();
	HandleTriggerableActionOnRemoved(ActionDefinition);
}

void UNexusActionComponent::HandleTriggerableActionOnAdded(const FNexusActionDef& NewActionDefinition)
{
	if (!HasActionTriggerAuthority(NewActionDefinition.ActionInstance))
	{
		return;
	}

	BindActionTriggerEvent(NewActionDefinition);
	if (NewActionDefinition.ActionInstance->ShouldTriggerOnAdded())
	{
		TryTriggerAction(NewActionDefinition.Handle, FNexusEventMessage());
	}
}

void UNexusActionComponent::HandleTriggerableActionOnRemoved(const FNexusActionDef& ActionDefinition)
{
	if (HasActionTriggerAuthority(ActionDefinition.ActionInstance))
	{
		UnbindActionTriggerEvent(ActionDefinition);
	}
}

void UNexusActionComponent::BindActionTriggerEvent(const FNexusActionDef& NewActionDefinition)
{
	UNexusAction* Action = NewActionDefinition.ActionInstance;

	FGameplayTagContainer ActionTriggerEventTags = Action->GetActionTriggerEventTags();
	UNexusEventManagerComponent* EventManagerComponent = AgentInfo->OwnerActor->GetComponentByClass<UNexusEventManagerComponent>();
	if (!EventManagerComponent)
	{
		return;
	}

	for (FGameplayTag Tag : ActionTriggerEventTags)
	{
		FNexusEventCallbackHandle EventCallbackHandle = EventManagerComponent->BindEventCallback<FNexusEventMessage>(Tag, this, &ThisClass::OnActionEventTriggered, NewActionDefinition.Handle);
		BoundedActionEventHandles.FindOrAdd(NewActionDefinition.Handle).Add(EventCallbackHandle);
	}
}

void UNexusActionComponent::UnbindActionTriggerEvent(const FNexusActionDef& ActionDefinition)
{
	if (TArray<FNexusEventCallbackHandle>* EventCallbackHandles = BoundedActionEventHandles.Find(ActionDefinition.Handle))
	{
		UNexusEventManagerComponent* EventManagerComponent = AgentInfo->OwnerActor->GetComponentByClass<UNexusEventManagerComponent>();
		if (!EventManagerComponent)
		{
			return;
		}

		for (FNexusEventCallbackHandle EventCallbackHandle : *EventCallbackHandles)
		{
			EventManagerComponent->UnbindEventCallback(EventCallbackHandle);
		}
		BoundedActionEventHandles.Remove(ActionDefinition.Handle);
	}
}

void UNexusActionComponent::OnActionEventTriggered(FGameplayTag GameplayTag, const FNexusEventMessage& EventMessage, FNexusActionDefHandle ActionDefinitionHandle)
{
	TryTriggerAction(ActionDefinitionHandle, EventMessage);
}

void UNexusActionComponent::OnActionEnded(FNexusActionDefHandle ActionDefinitionHandle, UNexusAction* Action)
{
	NX_LOG_SUB(LogNexusAction, Verbose, TEXT("Action [%s] 종료"), *Action->GetName());
	OwnedTags.RemoveTags(Action->GetActionOwnedTags());
}

FNexusActionDef* UNexusActionComponent::FindActionDefinitionByHandle(FNexusActionDefHandle ActionDefinitionHandle)
{
	check(ActionDefinitionHandle.IsValid());
	if (FNexusActionDef* ActionDefinitionPtr = ActionDefinitions.FindActionDefinitionByHandle(ActionDefinitionHandle))
	{
		return ActionDefinitionPtr;
	}

	// If not found in ActionDefinitions, try to find in ActionPendingAdds
	return ActionPendingAdds.FindByPredicate([ActionDefinitionHandle](const FNexusActionDef& ActionDefinition)
	{
		return ActionDefinition.Handle == ActionDefinitionHandle;
	});
}

bool UNexusActionComponent::CanTriggerAction(const FNexusActionDef& ActionDefinition) const
{
	bool bIsNotTriggeringOrRetriggerable = ActionDefinition.ActionInstance->IsRetriggerable() || !ActionDefinition.ActionInstance->IsTriggering();
	bool bMetTriggerCondition = ActionDefinition.ActionInstance->OnCanTriggerAction();
	bool bHasRequiredTags = OwnedTags.HasAll(ActionDefinition.ActionInstance->GetShouldHaveTags());
	bool bDontHaveForbiddenTags = !OwnedTags.HasAny(ActionDefinition.ActionInstance->GetShouldNotHaveTags());

	if (!bShowActionFailedReason)
	{
		return bIsNotTriggeringOrRetriggerable && bMetTriggerCondition && bHasRequiredTags && bDontHaveForbiddenTags;
	}

	if (!bIsNotTriggeringOrRetriggerable)
	{
		NX_LOG_SUB(LogNexusAction, Verbose, TEXT("Action [%s] 거부 (실행 중)"), *ActionDefinition.ActionInstance->GetName());
		return false;
	}

	if (!bMetTriggerCondition)
	{
		NX_LOG_SUB(LogNexusAction, Verbose, TEXT("Action [%s] 거부 (조건 미충족)"), *ActionDefinition.ActionInstance->GetName());
		return false;
	}

	if (!bHasRequiredTags)
	{
		NX_LOG_SUB(LogNexusAction, Verbose, TEXT("Action [%s] 거부 (필수 태그 미보유)"), *ActionDefinition.ActionInstance->GetName());
		const FGameplayTagContainer& ShouldHaveTags = ActionDefinition.ActionInstance->GetShouldHaveTags();
		FGameplayTagContainer NotOwnedTags;
		for (FGameplayTag Tag : ShouldHaveTags)
		{
			if (!OwnedTags.HasTag(Tag))
			{
				NotOwnedTags.AddTag(Tag);
			}
		}
		NX_LOG_SUB(LogNexusAction, Verbose, TEXT("미보유 태그: %s"), *NotOwnedTags.ToStringSimple(true));
		return false;
	}

	if (!bDontHaveForbiddenTags)
	{
		NX_LOG_SUB(LogNexusAction, Verbose, TEXT("Action [%s] 거부 (금지 태그 보유)"), *ActionDefinition.ActionInstance->GetName());
		const FGameplayTagContainer& ShouldNotHaveTags = ActionDefinition.ActionInstance->GetShouldNotHaveTags();
		FGameplayTagContainer OwnedForbiddenTags;
		for (FGameplayTag Tag : ShouldNotHaveTags)
		{
			if (OwnedTags.HasTag(Tag))
			{
				OwnedForbiddenTags.AddTag(Tag);
			}
		}
		NX_LOG_SUB(LogNexusAction, Verbose, TEXT("보유 금지 태그: %s"), *OwnedForbiddenTags.ToStringSimple(true));
		return false;
	}

	return true;
}

void UNexusActionComponent::LocalTriggerAction(FNexusActionDef* ActionDefinition, FNexusPredictionTag PredictionHandle)
{
	NX_LOG_SUB(LogNexusAction, Verbose, TEXT( "Action [%s] 실행" ), *ActionDefinition->ActionInstance->GetName());
	check(ActionDefinition->ActionInstance);
	OwnedTags.AppendTags(ActionDefinition->ActionInstance->GetActionOwnedTags());
	ActionDefinition->ActionInstance->InitPredictionHandle = PredictionHandle;
	FNexusPredictionScope ScopedNetPrediction(*this, GetOwner()->HasAuthority(), PredictionHandle);
	ActionDefinition->ActionInstance->OnTriggerAction();
}


void UNexusActionComponent::ClientTriggerActionRequestFailed_Implementation(FNexusActionDefHandle ActionDefinitionHandle, FNexusPredictionTag PredictionHandle)
{
	FNexusActionDef* ActionDefinition = FindActionDefinitionByHandle(ActionDefinitionHandle);
	check(ActionDefinition);
	ActionDefinition->ActionInstance->OnEndAction();
	FNexusPredictionEvents::BroadcastOnPredictionFailed(PredictionHandle);
}

void UNexusActionComponent::ClientTriggerActionRequestSucceeded_Implementation(FNexusActionDefHandle ActionDefinitionHandle, FNexusPredictionTag PredictionHandle)
{
}

void UNexusActionComponent::ServerRemoteRequestTryTriggerAction_Implementation(FNexusActionDefHandle ActionDefinitionHandle, const FNexusEventMessage& EventMessage)
{
	TryTriggerAction(ActionDefinitionHandle, EventMessage);
}

void UNexusActionComponent::ClientRemoteRequestTryTriggerAction_Implementation(FNexusActionDefHandle ActionDefinitionHandle, const FNexusEventMessage& EventMessage)
{
	TryTriggerAction(ActionDefinitionHandle, EventMessage);
}


void UNexusActionComponent::AuthAddProperty(FGameplayTag Tag, float Value)
{
	check(GetOwner()->HasAuthority());
	UNexusProperty* NewProperty = NewObject<UNexusProperty>(GetOwner());
	NewProperty->SetTag(Tag);
	NewProperty->SetStaticValue(Value);
	Properties.Add(NewProperty);
}

void UNexusActionComponent::AuthRemoveProperty(FGameplayTag Tag)
{
	check(GetOwner()->HasAuthority());
	Properties.RemoveAll([Tag](UNexusProperty* Property)
	{
		return Property->GetTag() == Tag;
	});
}

void UNexusActionComponent::AuthRemoveAllProperties()
{
	check(GetOwner()->HasAuthority());
	Properties.Empty();
}


UNexusProperty* UNexusActionComponent::GetProperty(FGameplayTag Tag)
{
	TObjectPtr<UNexusProperty>* PropertyPtr = Properties.FindByPredicate([Tag](UNexusProperty* Property)
	{
		return Property->GetTag() == Tag;
	});
	return PropertyPtr ? *PropertyPtr : nullptr;
}

void UNexusActionComponent::AddStaticOperation(FGameplayTag Tag, FNexusPropertyOperation Operation)
{
	TObjectPtr<UNexusProperty>* PropertyPtr = Properties.FindByPredicate([Tag](UNexusProperty* Property)
	{
		return Property->GetTag() == Tag;
	});

	if (PropertyPtr)
	{
		(*PropertyPtr)->AddStaticOperation(Operation);
	}
}

void UNexusActionComponent::AddDynamicOperation(FGameplayTag Tag, FNexusPropertyOperation Operation)
{
	TObjectPtr<UNexusProperty>* PropertyPtr = Properties.FindByPredicate([Tag](UNexusProperty* Property)
	{
		return Property->GetTag() == Tag;
	});

	if (PropertyPtr)
	{
		(*PropertyPtr)->AddDynamicOperation(Operation);
	}
}

void UNexusActionComponent::RemoveOperationByHandle(FGameplayTag Tag, const FNexusPropertyOperationHandle& OperationHandle)
{
	TObjectPtr<UNexusProperty>* PropertyPtr = Properties.FindByPredicate([Tag](UNexusProperty* Property)
	{
		return Property->GetTag() == Tag;
	});

	if (PropertyPtr)
	{
		(*PropertyPtr)->RemoveOperationByHandle(OperationHandle);
	}
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

void UNexusActionComponent::ClientTriggerAction_Implementation(FNexusActionDefHandle ActionDefinitionHandle, const FNexusEventMessageReplicated& EventMessageReplicated)
{
	check(ActionDefinitionHandle.IsValid());
	FNexusActionDef* ActionDefinition = FindActionDefinitionByHandle(ActionDefinitionHandle);
	check(ActionDefinition)
	ActionDefinition->ActionInstance->SetActionCurrentEventMessage(EventMessageReplicated.ToEventMessage());
	LocalTriggerAction(ActionDefinition);
}

void UNexusActionComponent::ServerTryTriggerAction_Implementation(FNexusActionDefHandle ActionDefinitionHandle, const FNexusEventMessageReplicated& EventMessageReplicated, FNexusPredictionTag PredictionHandle)
{
	if (!ActionDefinitionHandle.IsValid())
	{
		unimplemented(); // FAIL DESYNC
	}
	FNexusActionDef* ActionDefinition = FindActionDefinitionByHandle(ActionDefinitionHandle);
	if (!ActionDefinition)
	{
		unimplemented(); // FAIL DESYNC
	}

	LocalTriggerAction(ActionDefinition, PredictionHandle);
}
