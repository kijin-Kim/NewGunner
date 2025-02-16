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
	DOREPLIFETIME_CONDITION(UNexusActionComponent, ActionDefs, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UNexusActionComponent, NetPredictionTags, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UNexusActionComponent, SideEffectDefs, COND_OwnerOnly);
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
	SideEffectDefs.Init(GetOwner());
	ActionDefs.OnActionDefAddedDelegate.BindUObject(this, &UNexusActionComponent::OnActionDefAdded);
	ActionDefs.OnActionDefRemovedDelegate.BindUObject(this, &UNexusActionComponent::OnActionDefRemoved);


	if (GetOwner()->HasAuthority())
	{
		AuthRemoveAllActions();
	}

	if (OldAgentInfo != *AgentInfo && !GetOwner()->HasAuthority() && AgentInfo->IsLocallyControlled())
	{
		for (auto& ActionDef : ActionDefs.Items)
		{
			OnActionDefAdded(ActionDef);
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
	SideEffectDefs.Tick(DeltaTime);
}

FNexusActionDefHandle UNexusActionComponent::AuthAddAction(const FNexusActionDef& ActionDef)
{
	check(GetOwner()->HasAuthority());

	if (ActionScopeLockCount > 0)
	{
		ActionPendingAdds.Add(ActionDef);
		return ActionDef.Handle;
	}

	ACTION_LIST_SCOPE_LOCK();

	FNexusActionDef NewActionDef = ActionDef;
	NewActionDef.ActionInstance = NewAction<UNexusAction>(NewActionDef.ActionClass, NewActionDef.Handle, AgentInfo);
	ActionDefs.AuthAdd(NewActionDef);
	HandleTriggerableActionOnAdded(NewActionDef);

	return NewActionDef.Handle;
}

void UNexusActionComponent::AuthRemoveAction(const FNexusActionDefHandle& ActionDefHandle)
{
	check(GetOwner()->HasAuthority());


	FNexusActionDef* ActionDef = FindActionDefByHandle(ActionDefHandle);
	if (!ActionDef)
	{
		return;
	}
	NX_LOG_SUB(LogNexusAction, Verbose, TEXT("Action [%s] 제거"), *ActionDef->ActionInstance->GetName());

	if (ActionScopeLockCount > 0)
	{
		ActionPendingRemoves.Add(ActionDefHandle);
		return;
	}

	ActionDef->ActionInstance->EndAction();
	HandleTriggerableActionOnRemoved(*ActionDef);

	ACTION_LIST_SCOPE_LOCK();
	ActionDefs.AuthRemove(ActionDefHandle);
}

void UNexusActionComponent::AuthRemoveAllActions()
{
	check(GetOwner()->HasAuthority());
	ACTION_LIST_SCOPE_LOCK();
	for (const auto& ActionDef : ActionDefs.Items)
	{
		ActionDef.ActionInstance->EndAction();
		HandleTriggerableActionOnRemoved(ActionDef);
		NX_LOG_SUB(LogNexusAction, Verbose, TEXT("Action [%s] 제거"), *ActionDef.ActionInstance->GetName());
	}
	ActionDefs.AuthRemoveAll();
}

void UNexusActionComponent::TryTriggerAction(FNexusActionDefHandle ActionDefHandle, const FNexusEventMessage& EventMessage)
{
	check(ActionDefHandle.IsValid());
	FNexusActionDef* ActionDef = FindActionDefByHandle(ActionDefHandle);

	check(AgentInfo->OwnerActor.IsValid());
	check(ActionDef->ActionInstance);

	ensureMsgf(ActionDef, TEXT("AddAction을 통해 먼저 해당 Action을 부여해야 합니다"));
	ensureMsgf(AgentInfo->AgentActor->GetLocalRole() != ROLE_SimulatedProxy,
	           TEXT("Agent Actor: [%s]가 SimulatedProxy인 경우 Action [%s]을(를) 실행할 수 없습니다"), *AgentInfo->AgentActor->GetName(),
	           *ActionDef->ActionInstance->GetName());

	ActionDef->ActionInstance->SetActionCurrentEventMessage(EventMessage);
	if (!CanTriggerAction(*ActionDef))
	{
		return;
	}


	if (AgentInfo->OwnerActor->GetNetMode() == NM_Standalone)
	{
		LocalTriggerAction(ActionDef);
		return;
	}

	const bool bIsLocallyControlled = AgentInfo->IsLocallyControlled();
	const bool bIsOwnerActorAuthoritative = GetOwner()->HasAuthority();
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
			ClientTriggerAction(ActionDefHandle, EventMessage);
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
			CurrentPredictionTag.GenerateNewHandle();
			LocalTriggerAction(ActionDef, CurrentPredictionTag);
			ServerTryTriggerAction(ActionDefHandle, EventMessage, CurrentPredictionTag);
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

void UNexusActionComponent::ServerSendNetSyncPoint_Implementation(FNexusActionDefHandle Handle, FNexusPredictionTag InitPredictionTag, FNexusPredictionTag NewPredictionTag)
{
	int32 Index = NetSyncPointDelegates.Find(FNetSyncPointDelegate::SyncPointDelegateKeyType(Handle, InitPredictionTag));
	if (Index == INDEX_NONE)
	{
		NetSyncPointDelegates.Add(FNetSyncPointDelegate(FNetSyncPointDelegate::SyncPointDelegateKeyType(Handle, InitPredictionTag), NewPredictionTag));
		return;
	}

	if (NetSyncPointDelegates[Index].OnSyncDelegate.IsBound())
	{
		FNexusPredictionScope PredictionScope(*this, GetOwner()->HasAuthority(), NewPredictionTag);
		NetSyncPointDelegates[Index].OnSyncDelegate.Broadcast();
	}
	NetSyncPointDelegates.RemoveAt(Index);
}


void UNexusActionComponent::CallOrAddSNetyncPointDelegate(FNexusActionDefHandle Handle, FNexusPredictionTag InitPredictionTag, FSimpleMulticastDelegate::FDelegate&& Delegate)
{
	int32 Index = NetSyncPointDelegates.Find(FNetSyncPointDelegate::SyncPointDelegateKeyType(Handle, InitPredictionTag));
	if (Index == INDEX_NONE)
	{
		NetSyncPointDelegates.Add(FNetSyncPointDelegate(FNetSyncPointDelegate::SyncPointDelegateKeyType(Handle, InitPredictionTag), MoveTemp(Delegate)));
		return;
	}

	FNexusPredictionScope PredictionScope(*this, GetOwner()->HasAuthority(), NetSyncPointDelegates[Index].NewPredictionTag);
	Delegate.ExecuteIfBound();
	NetSyncPointDelegates.RemoveAt(Index);
}

void UNexusActionComponent::ReplicatedNetPredictionTag(const FNexusPredictionTag& PredictionTag)
{
	NetPredictionTags.ReplicatedNetPredictionTag(PredictionTag);
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

		for (const auto& ActionDef : PendingAdds)
		{
			if (GetOwner()->HasAuthority())
			{
				AuthAddAction(ActionDef);
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

FNexusSideEffectDef UNexusActionComponent::MakeSideEffectDef(UNexusAction* Action, TSubclassOf<UNexusSideEffect> SideEffectClass)
{
	check(Action);
	FNexusSideEffectDef NewSideEffectDef{SideEffectClass};
	NewSideEffectDef.SideEffectInstance = NewObject<UNexusSideEffect>(Action->GetOwnerActor(), SideEffectClass);
	return NewSideEffectDef;
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
	FNexusSideEffectDef NewSideEffectDef = MakeSideEffectDef(Action, SideEffectClass);
	TriggerSideEffectByDef(NewSideEffectDef, Action);
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

void UNexusActionComponent::TriggerSideEffectByDef(const FNexusSideEffectDef& NewSideEffectDef, UNexusAction* Action)
{
	check(Action);
	check(!CurrentPredictionTag.IsExpired());
	SideEffectDefs.Add(NewSideEffectDef, CurrentPredictionTag);

	if (!GetOwner()->HasAuthority() && Action->GetActionNetMethod() == ENexusActionNetMethod::LocalPredicted)
	{
		FNexusPredictionEvents::FPredictionEvent& PredictionEvent = FNexusPredictionEvents::GetPredictionEvent(CurrentPredictionTag);
		PredictionEvent.OnPredictionEnded.AddLambda([this, SideEffectDefHandle = NewSideEffectDef.Handle]()
		{
			NX_LOG_SUB(LogNexusSideEffect, Verbose, TEXT("SideEffect [%s] 삭제 (예측 종료)"), *SideEffectDefHandle.ToString());
			SideEffectDefs.Remove(SideEffectDefHandle);
		});

		PredictionEvent.OnPredictionFailed.AddLambda([this, SideEffectDefHandle = NewSideEffectDef.Handle]()
		{
			NX_LOG_SUB(LogNexusSideEffect, Error, TEXT("SideEffect [%s] 삭제 (예측 실패)"), *SideEffectDefHandle.ToString());
			SideEffectDefs.Remove(SideEffectDefHandle);
		});
	}
}

void UNexusActionComponent::BP_TriggerCue(UNexusAction* Action, TSubclassOf<UNexusCue> CueClass, FNexusRepDataHandle TargetData)
{
	check(Action);
	AActor* ActorOwner = Cast<AActor>(Action->GetOuter());
	check(ActorOwner);
	if (UNexusActionComponent* ActionComponent = GetActionComponentFromActor(ActorOwner))
	{
		ActionComponent->TriggerCue(Action, CueClass, TargetData);
	}
}

void UNexusActionComponent::TriggerCue(UNexusAction* Action, TSubclassOf<UNexusCue> CueClass, FNexusRepDataHandle TargetData)

{
	if (!CueClass || !Action)
	{
		return;
	}

	if (GetOwner()->HasAuthority())
	{
		NetMulticastTriggerCue(CueClass, CurrentPredictionTag, TargetData);
		return;
	}

	if (Action->GetActionNetMethod() != ENexusActionNetMethod::ServerAuthoritative)
	{
		InternalTriggerCue(CueClass, TargetData);
	}
}

void UNexusActionComponent::NetMulticastTriggerCue_Implementation(TSubclassOf<UNexusCue> CueClass, FNexusPredictionTag PredictionTag, FNexusRepDataHandle TargetData)
{
	if (!AgentInfo->IsLocallyControlled() || GetOwner()->HasAuthority() || (AgentInfo->IsLocallyControlled() && !PredictionTag.IsValid()))
	{
		InternalTriggerCue(CueClass, TargetData);
	}
}


void UNexusActionComponent::InternalTriggerCue(TSubclassOf<UNexusCue> CueClass, FNexusRepDataHandle TargetData)
{
	if (!CueClass)
	{
		return;
	}

	UNexusCue* Sign = CueClass.GetDefaultObject();
	NX_LOG_SUB(LogNexusSideEffect, Verbose, TEXT("Sign [%s] 실행"), *Sign->GetName());
	check(Sign);

	Sign->SetCueRepData(TargetData);
	Sign->OnTriggered();
	Sign->SetCueRepData(FNexusRepDataHandle());
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


		for (const FNexusSideEffectDef& Item : SideEffectDefs.Items)
		{
			DisplayDebugManager.DrawString(FString::Printf(TEXT("SideEffect: %s"), *Item.SideEffectClass->GetName()));
		}
	}
}

void UNexusActionComponent::OnActionDefAdded(FNexusActionDef& ActionDef)
{
	ActionDef.ActionInstance = NewAction<UNexusAction>(ActionDef.ActionClass, ActionDef.Handle, AgentInfo);
	HandleTriggerableActionOnAdded(ActionDef);
}

void UNexusActionComponent::OnActionDefRemoved(FNexusActionDef& ActionDef)
{
	ActionDef.ActionInstance->EndAction();
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
	UNexusEventManagerComponent* EventManagerComponent = AgentInfo->OwnerActor->GetComponentByClass<UNexusEventManagerComponent>();
	if (!EventManagerComponent)
	{
		return;
	}

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
		UNexusEventManagerComponent* EventManagerComponent = AgentInfo->OwnerActor->GetComponentByClass<UNexusEventManagerComponent>();
		if (!EventManagerComponent)
		{
			return;
		}

		for (FNexusEventCallbackHandle EventCallbackHandle : *EventCallbackHandles)
		{
			EventManagerComponent->UnbindEventCallback(EventCallbackHandle);
		}
		BoundedActionEventHandles.Remove(ActionDef.Handle);
	}
}

void UNexusActionComponent::OnActionEventTriggered(FGameplayTag GameplayTag, const FNexusEventMessage& EventMessage, FNexusActionDefHandle ActionDefHandle)
{
	TryTriggerAction(ActionDefHandle, EventMessage);
}

void UNexusActionComponent::OnActionEnded(FNexusActionDefHandle ActionDefHandle, UNexusAction* Action)
{
	NX_LOG_SUB(LogNexusAction, Verbose, TEXT("Action [%s] 종료"), *Action->GetName());
	OwnedTags.RemoveTags(Action->GetActionOwnedTags());
}

FNexusActionDef* UNexusActionComponent::FindActionDefByHandle(FNexusActionDefHandle ActionDefHandle)
{
	check(ActionDefHandle.IsValid());
	if (FNexusActionDef* ActionDefPtr = ActionDefs.FindActionDefByHandle(ActionDefHandle))
	{
		return ActionDefPtr;
	}

	// If not found in ActionDefs, try to find in ActionPendingAdds
	return ActionPendingAdds.FindByPredicate([ActionDefHandle](const FNexusActionDef& ActionDef)
	{
		return ActionDef.Handle == ActionDefHandle;
	});
}

bool UNexusActionComponent::CanTriggerAction(const FNexusActionDef& ActionDef) const
{
	bool bIsNotTriggeringOrRetriggerable = ActionDef.ActionInstance->IsRetriggerable() || !ActionDef.ActionInstance->IsTriggering();
	bool bMetTriggerCondition = ActionDef.ActionInstance->OnCanTriggerAction();
	bool bHasRequiredTags = OwnedTags.HasAll(ActionDef.ActionInstance->GetShouldHaveTags());
	bool bDontHaveForbiddenTags = !OwnedTags.HasAny(ActionDef.ActionInstance->GetShouldNotHaveTags());

	if (!bShowActionFailedReason)
	{
		return bIsNotTriggeringOrRetriggerable && bMetTriggerCondition && bHasRequiredTags && bDontHaveForbiddenTags;
	}

	if (!bIsNotTriggeringOrRetriggerable)
	{
		NX_LOG_SUB(LogNexusAction, Verbose, TEXT("Action [%s] 거부 (실행 중)"), *ActionDef.ActionInstance->GetName());
		return false;
	}

	if (!bMetTriggerCondition)
	{
		NX_LOG_SUB(LogNexusAction, Verbose, TEXT("Action [%s] 거부 (조건 미충족)"), *ActionDef.ActionInstance->GetName());
		return false;
	}

	if (!bHasRequiredTags)
	{
		NX_LOG_SUB(LogNexusAction, Verbose, TEXT("Action [%s] 거부 (필수 태그 미보유)"), *ActionDef.ActionInstance->GetName());
		const FGameplayTagContainer& ShouldHaveTags = ActionDef.ActionInstance->GetShouldHaveTags();
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
		NX_LOG_SUB(LogNexusAction, Verbose, TEXT("Action [%s] 거부 (금지 태그 보유)"), *ActionDef.ActionInstance->GetName());
		const FGameplayTagContainer& ShouldNotHaveTags = ActionDef.ActionInstance->GetShouldNotHaveTags();
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

void UNexusActionComponent::LocalTriggerAction(FNexusActionDef* ActionDef, FNexusPredictionTag PredictionTag)
{
	NX_LOG_SUB(LogNexusAction, Verbose, TEXT( "Action [%s] 실행" ), *ActionDef->ActionInstance->GetName());
	check(ActionDef->ActionInstance);
	OwnedTags.AppendTags(ActionDef->ActionInstance->GetActionOwnedTags());
	ActionDef->ActionInstance->InitPredictionTag = PredictionTag;
	FNexusPredictionScope PredictionScope(*this, GetOwner()->HasAuthority(), PredictionTag);
	ActionDef->ActionInstance->OnTriggerAction();
}


void UNexusActionComponent::ClientTriggerActionRequestFailed_Implementation(FNexusActionDefHandle ActionDefHandle, FNexusPredictionTag PredictionTag)
{
	FNexusActionDef* ActionDef = FindActionDefByHandle(ActionDefHandle);
	check(ActionDef);
	ActionDef->ActionInstance->OnEndAction();
	FNexusPredictionEvents::BroadcastOnPredictionFailed(PredictionTag);
}

void UNexusActionComponent::ClientTriggerActionRequestSucceeded_Implementation(FNexusActionDefHandle ActionDefHandle, FNexusPredictionTag PredictionTag)
{
}

void UNexusActionComponent::ServerRemoteRequestTryTriggerAction_Implementation(FNexusActionDefHandle ActionDefHandle, const FNexusEventMessage& EventMessage)
{
	TryTriggerAction(ActionDefHandle, EventMessage);
}

void UNexusActionComponent::ClientRemoteRequestTryTriggerAction_Implementation(FNexusActionDefHandle ActionDefHandle, const FNexusEventMessage& EventMessage)
{
	TryTriggerAction(ActionDefHandle, EventMessage);
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

void UNexusActionComponent::ClientTriggerAction_Implementation(FNexusActionDefHandle ActionDefHandle, const FNexusEventMessageReplicated& EventMessageReplicated)
{
	check(ActionDefHandle.IsValid());
	FNexusActionDef* ActionDef = FindActionDefByHandle(ActionDefHandle);
	check(ActionDef)
	ActionDef->ActionInstance->SetActionCurrentEventMessage(EventMessageReplicated.ToEventMessage());
	LocalTriggerAction(ActionDef);
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

	LocalTriggerAction(ActionDef, PredictionTag);
}
