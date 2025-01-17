// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionComponent.h"
#include "GunnerAction.h"
#include "GunnerActionComponentInterface.h"
#include "GunnerActionScopedNetPrediction.h"
#include "GunnerActionSideEffect.h"
#include "GunnerActionSideEffectDefinition.h"
#include "GunnerActionSign.h"
#include "Engine/ActorChannel.h"
#include "Engine/Canvas.h"
#include "GameFramework/HUD.h"

#include "Gunner/_Core/Event/GunnerEventManagerComponent.h"
#include "Gunner/_Core/Input/GunnerEventMessage.h"
#include "Net/UnrealNetwork.h"

bool bShowActionFailedReason = false;
FAutoConsoleVariableRef ActionSystemShowActionTriggerFailedReasonCmd(
	TEXT("ActionSystem.Debug.ActionTriggerFailedReason"),
	bShowActionFailedReason,
	TEXT("")
);


UGunnerActionComponent::UGunnerActionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	AgentInfo = MakeShared<FGunnerActionAgentInfo>();
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		AHUD::OnShowDebugInfo.AddStatic(&ThisClass::OnShowDebugInfo);
	}
}

void UGunnerActionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	FGunneractionNetPredictionEvents::Clear();
}

void UGunnerActionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UGunnerActionComponent, ActionDefinitions, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UGunnerActionComponent, NetPredictionHandles, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UGunnerActionComponent, SideEffectDefinitions, COND_OwnerOnly);
	DOREPLIFETIME(UGunnerActionComponent, Properties);
}


bool UGunnerActionComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	for (UGunnerActionProperty* Property : Properties)
	{
		bWroteSomething |= Channel->ReplicateSubobject(Property, *Bunch, *RepFlags);
	}
	return bWroteSomething;
}

void UGunnerActionComponent::InitActionComponent(AActor* InAgentActor)
{
	check(InAgentActor);
	FGunnerActionAgentInfo OldAgentInfo = *AgentInfo;
	AgentInfo->Init(GetOwner(), InAgentActor);
	SideEffectDefinitions.Init(GetOwner());
	ActionDefinitions.OnActionDefinitionAddedDelegate.BindUObject(this, &UGunnerActionComponent::OnActionDefinitionAdded);
	ActionDefinitions.OnActionDefinitionRemovedDelegate.BindUObject(this, &UGunnerActionComponent::OnActionDefinitionRemoved);


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

void UGunnerActionComponent::ReleaseActionComponent()
{
	if (GetOwner()->HasAuthority())
	{
		AuthRemoveAllActions();
		AuthRemoveAllProperties();
		OwnedTags.Reset();
	}
}

void UGunnerActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	for (UGunnerActionProperty* Property : Properties)
	{
		Property->Tick();
	}
	SideEffectDefinitions.Tick(DeltaTime);
}

FGunnerActionDefinitionHandle UGunnerActionComponent::AuthAddAction(const FGunnerActionDefinition& ActionDefinition)
{
	check(GetOwner()->HasAuthority());

	if (ActionScopeLockCount > 0)
	{
		ActionPendingAdds.Add(ActionDefinition);
		return ActionDefinition.Handle;
	}

	ACTION_LIST_SCOPE_LOCK();

	FGunnerActionDefinition NewActionDefinition = ActionDefinition;
	NewActionDefinition.ActionInstance = NewGunnerAction<UGunnerAction>(NewActionDefinition.ActionClass, NewActionDefinition.Handle, AgentInfo);
	ActionDefinitions.AuthAdd(NewActionDefinition);
	HandleTriggerableActionOnAdded(NewActionDefinition);

	return NewActionDefinition.Handle;
}

void UGunnerActionComponent::AuthRemoveAction(const FGunnerActionDefinitionHandle& ActionDefinitionHandle)
{
	check(GetOwner()->HasAuthority());
	if (ActionScopeLockCount > 0)
	{
		ActionPendingRemoves.Add(ActionDefinitionHandle);
		return;
	}

	FGunnerActionDefinition* ActionDefinition = FindActionDefinitionByHandle(ActionDefinitionHandle);
	if (!ActionDefinition)
	{
		return;
	}

	ActionDefinition->ActionInstance->EndAction();
	HandleTriggerableActionOnRemoved(*ActionDefinition);

	ACTION_LIST_SCOPE_LOCK();
	ActionDefinitions.AuthRemove(ActionDefinitionHandle);
}

void UGunnerActionComponent::AuthRemoveAllActions()
{
	check(GetOwner()->HasAuthority());

	ACTION_LIST_SCOPE_LOCK();
	for (const auto& ActionDefinition : ActionDefinitions.Items)
	{
		ActionDefinition.ActionInstance->EndAction();
		HandleTriggerableActionOnRemoved(ActionDefinition);
	}
	ActionDefinitions.AuthRemoveAll();
}

void UGunnerActionComponent::TryTriggerAction(FGunnerActionDefinitionHandle ActionDefinitionHandle, const FGunnerEventMessage& EventMessage)
{
	check(ActionDefinitionHandle.IsValid());
	FGunnerActionDefinition* ActionDefinition = FindActionDefinitionByHandle(ActionDefinitionHandle);

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
	const EGunnerActionNetMethod ActionNetMethod = ActionDefinition->ActionInstance->GetActionNetMethod();
	const bool bIsRemoteTriggerable = ActionDefinition->ActionInstance->IsRemoteTriggerable();


	if (bIsOwnerActorAuthoritative)
	{
		if (bIsLocallyControlled || ActionNetMethod == EGunnerActionNetMethod::ServerOnly)
		{
			LocalTriggerAction(ActionDefinition);
			return;
		}

		if (ActionNetMethod == EGunnerActionNetMethod::ServerAuthoritative)
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
		if (ActionNetMethod == EGunnerActionNetMethod::LocalOnly)
		{
			LocalTriggerAction(ActionDefinition);
			return;
		}

		if (ActionNetMethod == EGunnerActionNetMethod::LocalPredicted)
		{
			CurrentNetPredictionHandle.GenerateNewHandle();
			LocalTriggerAction(ActionDefinition, CurrentNetPredictionHandle);
			ServerTryTriggerAction(ActionDefinitionHandle, EventMessage, CurrentNetPredictionHandle);
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

void UGunnerActionComponent::ServerSendNetSyncPoint_Implementation(FGunnerActionDefinitionHandle Handle, FGunnerActionNetPredictionHandle InitPredictionHandle, FGunnerActionNetPredictionHandle NewPredictionHandle)
{
	int32 Index = NetSyncPointDelegates.Find(FNetSyncPointDelegate::SyncPointDelegateKeyType(Handle, InitPredictionHandle));
	if (Index == INDEX_NONE)
	{
		NetSyncPointDelegates.Add(FNetSyncPointDelegate(FNetSyncPointDelegate::SyncPointDelegateKeyType(Handle, InitPredictionHandle), NewPredictionHandle));
		return;
	}

	if (NetSyncPointDelegates[Index].OnSyncDelegate.IsBound())
	{
		FGunnerActionScopedNetPrediction ScopedNetPrediction(*this, GetOwner()->HasAuthority(), NewPredictionHandle);
		NetSyncPointDelegates[Index].OnSyncDelegate.Broadcast();
	}
	NetSyncPointDelegates.RemoveAt(Index);
}


void UGunnerActionComponent::CallOrAddSNetyncPointDelegate(FGunnerActionDefinitionHandle Handle, FGunnerActionNetPredictionHandle InitPredictionHandle, FSimpleMulticastDelegate::FDelegate&& Delegate)
{
	int32 Index = NetSyncPointDelegates.Find(FNetSyncPointDelegate::SyncPointDelegateKeyType(Handle, InitPredictionHandle));
	if (Index == INDEX_NONE)
	{
		NetSyncPointDelegates.Add(FNetSyncPointDelegate(FNetSyncPointDelegate::SyncPointDelegateKeyType(Handle, InitPredictionHandle), MoveTemp(Delegate)));
		return;
	}

	FGunnerActionScopedNetPrediction ScopedNetPrediction(*this, GetOwner()->HasAuthority(), NetSyncPointDelegates[Index].NewPredictionHandle);
	Delegate.ExecuteIfBound();
	NetSyncPointDelegates.RemoveAt(Index);
}

void UGunnerActionComponent::ReplicatedNetPredictionHandle(const FGunnerActionNetPredictionHandle& PredictionHandle)
{
	NetPredictionHandles.ReplicatedNetPredictionHandle(PredictionHandle);
}

void UGunnerActionComponent::IncreaseActionListLock()
{
	ActionScopeLockCount++;
}

void UGunnerActionComponent::DecreaseActionListLock()
{
	ActionScopeLockCount--;
	if (ActionScopeLockCount == 0 && (ActionPendingAdds.IsEmpty() || ActionPendingRemoves.IsEmpty()))
	{
		TArray<FGunnerActionDefinition> PendingAdds = MoveTemp(ActionPendingAdds);
		TArray<FGunnerActionDefinitionHandle> PendingRemoves = MoveTemp(ActionPendingRemoves);

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

UGunnerActionComponent* UGunnerActionComponent::GetActionComponentFromActor(AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}

	if (UGunnerActionComponent* ActionComponent = Actor->GetComponentByClass<UGunnerActionComponent>())
	{
		return ActionComponent;
	}

	if (IGunnerActionComponentInterface* GunnerActionComponentInterface = Cast<IGunnerActionComponentInterface>(Actor))
	{
		return GunnerActionComponentInterface->GetActionComponent();
	}

	return nullptr;
}

bool UGunnerActionComponent::HasActionTriggerAuthority(UGunnerAction* Action) const
{
	check(Action);
	EGunnerActionNetMethod ActionNetMethod = Action->GetActionNetMethod();
	if (ActionNetMethod == EGunnerActionNetMethod::LocalOnly || ActionNetMethod == EGunnerActionNetMethod::LocalPredicted)
	{
		return AgentInfo->IsLocallyControlled() || Action->IsRemoteTriggerable();
	}

	if (ActionNetMethod == EGunnerActionNetMethod::ServerOnly || ActionNetMethod == EGunnerActionNetMethod::ServerAuthoritative)
	{
		return GetOwner()->HasAuthority() || Action->IsRemoteTriggerable();
	}

	return false;
}

FGunnerActionSideEffectDefinition UGunnerActionComponent::MakeSideEffectDefinition(UGunnerAction* Action, TSubclassOf<UGunnerActionSideEffect> SideEffectClass)
{
	check(Action);
	FGunnerActionSideEffectDefinition NewSideEffectDefinition{SideEffectClass};
	NewSideEffectDefinition.SideEffectInstance = NewObject<UGunnerActionSideEffect>(Action->GetOwnerActor(), SideEffectClass);
	return NewSideEffectDefinition;
}

void UGunnerActionComponent::BP_TriggerSideEffectToActor(UGunnerAction* Action, AActor* SideEffectTarget, TSubclassOf<UGunnerActionSideEffect> SideEffectClass)
{
	check(Action);
	if (!SideEffectTarget)
	{
		return;
	}

	if (UGunnerActionComponent* ActionComponent = GetActionComponentFromActor(SideEffectTarget))
	{
		ActionComponent->TriggerSideEffect(SideEffectClass, Action);
	}
}

void UGunnerActionComponent::TriggerSideEffect(TSubclassOf<UGunnerActionSideEffect> SideEffectClass, UGunnerAction* Action)
{
	check(Action);
	FGunnerActionSideEffectDefinition NewSideEffectDefinition = MakeSideEffectDefinition(Action, SideEffectClass);
	TriggerSideEffectByDefinition(NewSideEffectDefinition, Action);
}

void UGunnerActionComponent::BP_TriggerSideEffectToActorByDefinition(UGunnerAction* Action, AActor* SideEffectTarget, const FGunnerActionSideEffectDefinition& SideEffectDefinition)
{
	check(Action);
	if (!SideEffectTarget)
	{
		return;
	}

	if (UGunnerActionComponent* ActionComponent = GetActionComponentFromActor(SideEffectTarget))
	{
		ActionComponent->TriggerSideEffectByDefinition(SideEffectDefinition, Action);
	}
}

void UGunnerActionComponent::TriggerSideEffectByDefinition(const FGunnerActionSideEffectDefinition& NewSideEffectDefinition, UGunnerAction* Action)
{
	check(Action);
	check(!CurrentNetPredictionHandle.IsExpired());
	SideEffectDefinitions.Add(NewSideEffectDefinition, CurrentNetPredictionHandle);

	if (!GetOwner()->HasAuthority() && Action->GetActionNetMethod() == EGunnerActionNetMethod::LocalPredicted)
	{
		FGunneractionNetPredictionEvents::FPredictionEvent& PredictionEvent = FGunneractionNetPredictionEvents::GetPredictionEvent(CurrentNetPredictionHandle);
		PredictionEvent.OnPredictionEnded.AddLambda([this, SideEffectDefinitionHandle = NewSideEffectDefinition.Handle]()
		{
			GR_LOG_SUB(LogGunnerSideEffect, Verbose, TEXT("SideEffect [%s] 삭제 (예측 종료)"), *SideEffectDefinitionHandle.ToString());
			SideEffectDefinitions.Remove(SideEffectDefinitionHandle);
		});

		PredictionEvent.OnPredictionFailed.AddLambda([this, SideEffectDefinitionHandle = NewSideEffectDefinition.Handle]()
		{
			GR_LOG_SUB(LogGunnerSideEffect, Error, TEXT("SideEffect [%s] 삭제 (예측 실패)"), *SideEffectDefinitionHandle.ToString());
			SideEffectDefinitions.Remove(SideEffectDefinitionHandle);
		});
	}
}

void UGunnerActionComponent::BP_Signal(UGunnerAction* Action, TSubclassOf<UGunnerActionSign> SignClass, UObject* SignalDataObject)
{
	check(Action);
	AActor* ActorOwner = Cast<AActor>(Action->GetOuter());
	check(ActorOwner);
	if (UGunnerActionComponent* ActionComponent = GetActionComponentFromActor(ActorOwner))
	{
		ActionComponent->Signal(Action, SignClass, SignalDataObject);
	}
}

void UGunnerActionComponent::Signal(UGunnerAction* Action, TSubclassOf<UGunnerActionSign> SignClass, UObject* SignalDataObject)
{
	if (!SignClass || !Action)
	{
		return;
	}

	if (GetOwner()->HasAuthority())
	{
		NetMulticastSignal(SignClass, SignalDataObject, CurrentNetPredictionHandle);
		return;
	}

	if (Action->GetActionNetMethod() != EGunnerActionNetMethod::ServerAuthoritative)
	{
		InternalSignal(SignClass, SignalDataObject);
	}
}

void UGunnerActionComponent::NetMulticastSignal_Implementation(TSubclassOf<UGunnerActionSign> SignClass, UObject* SignalDataObject, FGunnerActionNetPredictionHandle PredictionHandle)
{
	if (!AgentInfo->IsLocallyControlled() || GetOwner()->HasAuthority() || (AgentInfo->IsLocallyControlled() && !PredictionHandle.IsValid()))
	{
		InternalSignal(SignClass, SignalDataObject);
	}
}

void UGunnerActionComponent::InternalSignal(TSubclassOf<UGunnerActionSign> SignClass, UObject* SignalDataObject)
{
	UGunnerActionSign* Sign = SignClass.GetDefaultObject();
	GR_LOG_SUB(LogGunnerSignal, Verbose, TEXT("Sign [%s] 실행"), *Sign->GetName());
	check(Sign);
	Sign->SetSignalDataObject(SignalDataObject);
	Sign->OnSignaled();
	Sign->SetSignalDataObject(nullptr);
}

void UGunnerActionComponent::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y)
{
	AActor* DebugTarget = HUD->GetCurrentDebugTargetActor();
	if (!DebugTarget)
	{
		return;
	}

	if (UGunnerActionComponent* ActionComponent = GetActionComponentFromActor(DebugTarget))
	{
		ActionComponent->InternalOnShowDebugInfo(DebugTarget, HUD, Canvas, DebugDisplayInfo, X, Y);
	}
}

void UGunnerActionComponent::InternalOnShowDebugInfo(AActor* DebugTarget, AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y)
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


		for (UGunnerActionProperty* Property : Properties)
		{
			DisplayDebugManager.DrawString(FString::Printf(TEXT("%s: StaticValue: %f, DynamicValue: %f"), *Property->GetTag().ToString(), Property->GetStaticValue(), Property->GetDynamicValue()));
		}


		for (const FGunnerActionSideEffectDefinition& Item : SideEffectDefinitions.Items)
		{
			DisplayDebugManager.DrawString(FString::Printf(TEXT("SideEffect: %s"), *Item.SideEffectClass->GetName()));
		}
	}
}

void UGunnerActionComponent::OnActionDefinitionAdded(FGunnerActionDefinition& ActionDefinition)
{
	ActionDefinition.ActionInstance = NewGunnerAction<UGunnerAction>(ActionDefinition.ActionClass, ActionDefinition.Handle, AgentInfo);
	HandleTriggerableActionOnAdded(ActionDefinition);
}

void UGunnerActionComponent::OnActionDefinitionRemoved(FGunnerActionDefinition& ActionDefinition)
{
	ActionDefinition.ActionInstance->EndAction();
	HandleTriggerableActionOnRemoved(ActionDefinition);
}

void UGunnerActionComponent::HandleTriggerableActionOnAdded(const FGunnerActionDefinition& NewActionDefinition)
{
	if (!HasActionTriggerAuthority(NewActionDefinition.ActionInstance))
	{
		return;
	}

	BindActionTriggerEvent(NewActionDefinition);
	if (NewActionDefinition.ActionInstance->ShouldTriggerOnAdded())
	{
		TryTriggerAction(NewActionDefinition.Handle, FGunnerEventMessage());
	}
}

void UGunnerActionComponent::HandleTriggerableActionOnRemoved(const FGunnerActionDefinition& ActionDefinition)
{
	if (HasActionTriggerAuthority(ActionDefinition.ActionInstance))
	{
		UnbindActionTriggerEvent(ActionDefinition);
	}
}

void UGunnerActionComponent::BindActionTriggerEvent(const FGunnerActionDefinition& NewActionDefinition)
{
	UGunnerAction* Action = NewActionDefinition.ActionInstance;

	FGameplayTagContainer ActionTriggerEventTags = Action->GetActionTriggerEventTags();
	UGunnerEventManagerComponent* EventManagerComponent = AgentInfo->OwnerActor->GetComponentByClass<UGunnerEventManagerComponent>();
	if (!EventManagerComponent)
	{
		return;
	}

	for (FGameplayTag Tag : ActionTriggerEventTags)
	{
		FGunnerEventCallbackHandle EventCallbackHandle = EventManagerComponent->BindEventCallback<FGunnerEventMessage>(Tag, this, &ThisClass::OnActionEventTriggered, NewActionDefinition.Handle);
		BoundedActionEventHandles.FindOrAdd(NewActionDefinition.Handle).Add(EventCallbackHandle);
	}
}

void UGunnerActionComponent::UnbindActionTriggerEvent(const FGunnerActionDefinition& ActionDefinition)
{
	if (TArray<FGunnerEventCallbackHandle>* EventCallbackHandles = BoundedActionEventHandles.Find(ActionDefinition.Handle))
	{
		UGunnerEventManagerComponent* EventManagerComponent = AgentInfo->OwnerActor->GetComponentByClass<UGunnerEventManagerComponent>();
		if (!EventManagerComponent)
		{
			return;
		}

		for (FGunnerEventCallbackHandle EventCallbackHandle : *EventCallbackHandles)
		{
			EventManagerComponent->UnbindEventCallback(EventCallbackHandle);
		}
		BoundedActionEventHandles.Remove(ActionDefinition.Handle);
	}
}

void UGunnerActionComponent::OnActionEventTriggered(FGameplayTag GameplayTag, const FGunnerEventMessage& EventMessage, FGunnerActionDefinitionHandle ActionDefinitionHandle)
{
	TryTriggerAction(ActionDefinitionHandle, EventMessage);
}

void UGunnerActionComponent::OnActionEnded(FGunnerActionDefinitionHandle ActionDefinitionHandle, UGunnerAction* Action)
{
	GR_LOG_SUB(LogGunnerAction, Verbose, TEXT("Action [%s] 종료"), *Action->GetName());
	OwnedTags.RemoveTags(Action->GetActionOwnedTags());
}

FGunnerActionDefinition* UGunnerActionComponent::FindActionDefinitionByHandle(FGunnerActionDefinitionHandle ActionDefinitionHandle)
{
	check(ActionDefinitionHandle.IsValid());
	if (FGunnerActionDefinition* ActionDefinitionPtr = ActionDefinitions.FindActionDefinitionByHandle(ActionDefinitionHandle))
	{
		return ActionDefinitionPtr;
	}

	// If not found in ActionDefinitions, try to find in ActionPendingAdds
	return ActionPendingAdds.FindByPredicate([ActionDefinitionHandle](const FGunnerActionDefinition& ActionDefinition)
	{
		return ActionDefinition.Handle == ActionDefinitionHandle;
	});
}

bool UGunnerActionComponent::CanTriggerAction(const FGunnerActionDefinition& ActionDefinition) const
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
		GR_LOG_SUB(LogGunnerAction, Verbose, TEXT("Action [%s] 거부 (실행 중)"), *ActionDefinition.ActionInstance->GetName());
		return false;
	}

	if (!bMetTriggerCondition)
	{
		GR_LOG_SUB(LogGunnerAction, Verbose, TEXT("Action [%s] 거부 (조건 미충족)"), *ActionDefinition.ActionInstance->GetName());
		return false;
	}

	if (!bHasRequiredTags)
	{
		GR_LOG_SUB(LogGunnerAction, Verbose, TEXT("Action [%s] 거부 (필수 태그 미보유)"), *ActionDefinition.ActionInstance->GetName());
		const FGameplayTagContainer& ShouldHaveTags = ActionDefinition.ActionInstance->GetShouldHaveTags();
		FGameplayTagContainer NotOwnedTags;
		for (FGameplayTag Tag : ShouldHaveTags)
		{
			if (!OwnedTags.HasTag(Tag))
			{
				NotOwnedTags.AddTag(Tag);
			}
		}
		GR_LOG_SUB(LogGunnerAction, Verbose, TEXT("미보유 태그: %s"), *NotOwnedTags.ToStringSimple(true));
		return false;
	}

	if (!bDontHaveForbiddenTags)
	{
		GR_LOG_SUB(LogGunnerAction, Verbose, TEXT("Action [%s] 거부 (금지 태그 보유)"), *ActionDefinition.ActionInstance->GetName());
		const FGameplayTagContainer& ShouldNotHaveTags = ActionDefinition.ActionInstance->GetShouldNotHaveTags();
		FGameplayTagContainer OwnedForbiddenTags;
		for (FGameplayTag Tag : ShouldNotHaveTags)
		{
			if (OwnedTags.HasTag(Tag))
			{
				OwnedForbiddenTags.AddTag(Tag);
			}
		}
		GR_LOG_SUB(LogGunnerAction, Verbose, TEXT("보유 금지 태그: %s"), *OwnedForbiddenTags.ToStringSimple(true));
		return false;
	}

	return true;
}

void UGunnerActionComponent::LocalTriggerAction(FGunnerActionDefinition* ActionDefinition, FGunnerActionNetPredictionHandle PredictionHandle /*= FGunnerActionNetPredictionHandle()*/)
{
	GR_LOG_SUB(LogGunnerAction, Verbose, TEXT( "Action [%s] 실행" ), *ActionDefinition->ActionInstance->GetName());
	check(ActionDefinition->ActionInstance);
	OwnedTags.AppendTags(ActionDefinition->ActionInstance->GetActionOwnedTags());
	ActionDefinition->ActionInstance->InitPredictionHandle = PredictionHandle;
	FGunnerActionScopedNetPrediction ScopedNetPrediction(*this, GetOwner()->HasAuthority(), PredictionHandle);
	ActionDefinition->ActionInstance->OnTriggerAction();
}


void UGunnerActionComponent::ClientTriggerActionRequestFailed_Implementation(FGunnerActionDefinitionHandle ActionDefinitionHandle, FGunnerActionNetPredictionHandle PredictionHandle)
{
	FGunnerActionDefinition* ActionDefinition = FindActionDefinitionByHandle(ActionDefinitionHandle);
	check(ActionDefinition);
	ActionDefinition->ActionInstance->OnEndAction();
	FGunneractionNetPredictionEvents::BroadcastOnPredictionFailed(PredictionHandle);
}

void UGunnerActionComponent::ClientTriggerActionRequestSucceeded_Implementation(FGunnerActionDefinitionHandle ActionDefinitionHandle, FGunnerActionNetPredictionHandle PredictionHandle)
{
}

void UGunnerActionComponent::ServerRemoteRequestTryTriggerAction_Implementation(FGunnerActionDefinitionHandle ActionDefinitionHandle, const FGunnerEventMessage& EventMessage)
{
	TryTriggerAction(ActionDefinitionHandle, EventMessage);
}

void UGunnerActionComponent::ClientRemoteRequestTryTriggerAction_Implementation(FGunnerActionDefinitionHandle ActionDefinitionHandle, const FGunnerEventMessage& EventMessage)
{
	TryTriggerAction(ActionDefinitionHandle, EventMessage);
}


void UGunnerActionComponent::AuthAddProperty(FGameplayTag Tag, float Value)
{
	check(GetOwner()->HasAuthority());
	UGunnerActionProperty* NewProperty = NewObject<UGunnerActionProperty>(GetOwner());
	NewProperty->SetTag(Tag);
	NewProperty->SetStaticValue(Value);
	Properties.Add(NewProperty);
}

void UGunnerActionComponent::AuthRemoveProperty(FGameplayTag Tag)
{
	check(GetOwner()->HasAuthority());
	Properties.RemoveAll([Tag](UGunnerActionProperty* Property)
	{
		return Property->GetTag() == Tag;
	});
}

void UGunnerActionComponent::AuthRemoveAllProperties()
{
	check(GetOwner()->HasAuthority());
	Properties.Empty();
}


UGunnerActionProperty* UGunnerActionComponent::GetProperty(FGameplayTag Tag)
{
	TObjectPtr<UGunnerActionProperty>* PropertyPtr = Properties.FindByPredicate([Tag](UGunnerActionProperty* Property)
	{
		return Property->GetTag() == Tag;
	});
	return PropertyPtr ? *PropertyPtr : nullptr;
}

void UGunnerActionComponent::AddStaticOperation(FGameplayTag Tag, FGunnerActionPropertyOperation Operation)
{
	TObjectPtr<UGunnerActionProperty>* PropertyPtr = Properties.FindByPredicate([Tag](UGunnerActionProperty* Property)
	{
		return Property->GetTag() == Tag;
	});

	if (PropertyPtr)
	{
		(*PropertyPtr)->AddStaticOperation(Operation);
	}
}

void UGunnerActionComponent::AddDynamicOperation(FGameplayTag Tag, FGunnerActionPropertyOperation Operation)
{
	TObjectPtr<UGunnerActionProperty>* PropertyPtr = Properties.FindByPredicate([Tag](UGunnerActionProperty* Property)
	{
		return Property->GetTag() == Tag;
	});

	if (PropertyPtr)
	{
		(*PropertyPtr)->AddDynamicOperation(Operation);
	}
}

void UGunnerActionComponent::RemoveOperationByHandle(FGameplayTag Tag, const FGunnerActionPropertyOperationHandle& OperationHandle)
{
	TObjectPtr<UGunnerActionProperty>* PropertyPtr = Properties.FindByPredicate([Tag](UGunnerActionProperty* Property)
	{
		return Property->GetTag() == Tag;
	});

	if (PropertyPtr)
	{
		(*PropertyPtr)->RemoveOperationByHandle(OperationHandle);
	}
}

UGunnerActionProperty* UGunnerActionComponent::GetPropertyFromActor(AActor* Actor, FGameplayTag Tag)
{
	if (!Actor)
	{
		return nullptr;
	}

	UGunnerActionComponent* ActionComponent = GetActionComponentFromActor(Actor);
	if (!ActionComponent)
	{
		return nullptr;
	}

	return ActionComponent->GetProperty(Tag);
}

float UGunnerActionComponent::GetPropertyValueFromActor(AActor* Actor, FGameplayTag Tag)
{
	if (!Actor)
	{
		return 0.0f;
	}

	UGunnerActionComponent* ActionComponent = GetActionComponentFromActor(Actor);
	if (!ActionComponent)
	{
		return 0.0f;
	}

	UGunnerActionProperty* PropertyPtr = ActionComponent->GetProperty(Tag);
	if (!PropertyPtr)
	{
		return 0.0f;
	}

	return PropertyPtr->GetDynamicValue();
}

void UGunnerActionComponent::ClientTriggerAction_Implementation(FGunnerActionDefinitionHandle ActionDefinitionHandle, const FGunnerEventMessageReplicated& EventMessageReplicated)
{
	check(ActionDefinitionHandle.IsValid());
	FGunnerActionDefinition* ActionDefinition = FindActionDefinitionByHandle(ActionDefinitionHandle);
	check(ActionDefinition)
	ActionDefinition->ActionInstance->SetActionCurrentEventMessage(EventMessageReplicated.ToEventMessage());
	LocalTriggerAction(ActionDefinition);
}

void UGunnerActionComponent::ServerTryTriggerAction_Implementation(FGunnerActionDefinitionHandle ActionDefinitionHandle, const FGunnerEventMessageReplicated& EventMessageReplicated, FGunnerActionNetPredictionHandle PredictionHandle)
{
	if (!ActionDefinitionHandle.IsValid())
	{
		unimplemented(); // FAIL DESYNC
	}
	FGunnerActionDefinition* ActionDefinition = FindActionDefinitionByHandle(ActionDefinitionHandle);
	if (!ActionDefinition)
	{
		unimplemented(); // FAIL DESYNC
	}

	LocalTriggerAction(ActionDefinition, PredictionHandle);
}
