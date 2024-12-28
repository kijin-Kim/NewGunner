// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionComponent.h"
#include "GunnerAction.h"
#include "GunnerActionComponentInterface.h"
#include "GunnerActionScopedNetPrediction.h"
#include "GunnerActionSideEffect.h"
#include "GunnerActionSideEffectDefinition.h"
#include "GunnerActionSign.h"
#include "Engine/Canvas.h"
#include "GameFramework/HUD.h"

#include "Gunner/Gunner.h"
#include "Gunner/_Core/Event/GunnerEventManagerComponent.h"
#include "Gunner/_Core/Input/GunnerEventMessage.h"
#include "Net/UnrealNetwork.h"


UGunnerActionComponent::UGunnerActionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	AgentInfo = MakeShared<FGunnerActionAgentInfo>();
	bReplicateUsingRegisteredSubObjectList = true;
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		AHUD::OnShowDebugInfo.AddStatic(&ThisClass::OnShowDebugInfo);
	}
}

void UGunnerActionComponent::InitActionComponent(AActor* InOwnerActor, AActor* InAgentActor)
{
	FGunnerActionAgentInfo OldAgentInfo = *AgentInfo;
	AgentInfo->Init(InOwnerActor, InAgentActor);
	SideEffectDefinitionArray.OnSideEffectDefinitionAddedDelegate.BindUObject(this, &UGunnerActionComponent::OnAdded);
	SideEffectDefinitionArray.OnSideEffectDefinitionRemovedDelegate.BindUObject(this, &UGunnerActionComponent::OnRemoved);
	AuthRemoveAllActions();

	if (OldAgentInfo != *AgentInfo && !AgentInfo->IsOwnerActorAuthoritative() && AgentInfo->IsLocallyControlled())
	{
		OnRep_ActionDefinitions({});
	}
}

void UGunnerActionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UGunnerActionComponent, ActionDefinitions, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UGunnerActionComponent, NetPredictionHandleArray, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UGunnerActionComponent, SideEffectDefinitionArray, COND_OwnerOnly);
	DOREPLIFETIME(UGunnerActionComponent, PropertyArray);
}

void UGunnerActionComponent::ReadyForReplication()
{
	Super::ReadyForReplication();
	for (const auto& [Tag, Value] : StartProperties)
	{
		AuthAddProperty(Tag, Value);
	}
}

void UGunnerActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

FGunnerActionDefinitionHandle UGunnerActionComponent::AuthAddAction(const FGunnerActionDefinition& ActionDefinition)
{
	if (!AgentInfo->IsOwnerActorAuthoritative())
	{
		return FGunnerActionDefinitionHandle();
	}

	if (ActionScopeLockCount > 0)
	{
		ActionPendingAdds.Add(ActionDefinition);
		return ActionDefinition.Handle;
	}

	ACTION_LIST_SCOPE_LOCK();
	auto& NewActionDefinition = ActionDefinitions[ActionDefinitions.Add(ActionDefinition)];
	NewActionDefinition.ActionInstance = NewGunnerAction<UGunnerAction>(GetOwner(), NewActionDefinition.ActionClass, NewActionDefinition.Handle, AgentInfo);
	HandleTriggerableActionOnAdded(NewActionDefinition);
	return NewActionDefinition.Handle;
}

void UGunnerActionComponent::AuthRemoveAction(const FGunnerActionDefinitionHandle& ActionDefinitionHandle)
{
	if (!AgentInfo->IsOwnerActorAuthoritative())
	{
		return;
	}

	FGunnerActionDefinition* ActionDefinition = FindActionDefinitionByHandle(ActionDefinitionHandle);
	if (!ActionDefinition)
	{
		return;
	}

	ActionDefinition->ActionInstance->EndAction();
	HandleTriggerableActionOnRemoved(*ActionDefinition);

	if (ActionScopeLockCount > 0)
	{
		ActionPendingRemoves.Add(ActionDefinitionHandle);
		return;
	}

	ACTION_LIST_SCOPE_LOCK();
	ActionDefinitions.RemoveAll([ActionDefinitionHandle](const FGunnerActionDefinition& ActionDefinition)
	{
		return ActionDefinition.Handle == ActionDefinitionHandle;
	});
}

void UGunnerActionComponent::AuthRemoveAllActions()
{
	if (!AgentInfo->IsOwnerActorAuthoritative())
	{
		return;
	}

	ACTION_LIST_SCOPE_LOCK();
	for (const auto& ActionDefinition : ActionDefinitions)
	{
		AuthRemoveAction(ActionDefinition.Handle);
	}
}

void UGunnerActionComponent::TryTriggerAction(FGunnerActionDefinitionHandle ActionDefinitionHandle, const FGunnerEventMessage& EventMessage)
{
	check(ActionDefinitionHandle.IsValid());
	FGunnerActionDefinition* ActionDefinition = FindActionDefinitionByHandle(ActionDefinitionHandle);
	if (!ActionDefinition)
	{
		GR_LOG_SUB(LogGunner, Warning, TEXT("AddAction을 통해 먼저 해당 Action을 부여해야 합니다"));
		return;
	}

	if (!AgentInfo->OwnerActor.IsValid())
	{
		GR_LOG_SUB(LogGunner, Warning, TEXT("Owner Actor가 존재하지 않습니다."));
		return;
	}

	if (AgentInfo->AgentActor->GetLocalRole() == ROLE_SimulatedProxy)
	{
		GR_LOG_SUB(LogGunner, Error, TEXT("Agent Actor: [%s]가 SimulatedProxy인 경우 Action [%s]을(를) 실행할 수 없습니다."), *AgentInfo->AgentActor->GetName(), *ActionDefinition->ActionClass->GetName());
		return;
	}

	check(ActionDefinition->ActionInstance);
	if (!CanTriggerAction(*ActionDefinition, EventMessage))
	{
		GR_LOG_SUB(LogGunner, Error, TEXT("Action [%s]을(를) 실행이 거부되었습니다"), *ActionDefinition->ActionClass->GetName());
		return;
	}


	if (AgentInfo->OwnerActor->GetNetMode() == NM_Standalone)
	{
		LocalTriggerAction(ActionDefinition);
		return;
	}

	const bool bIsLocallyControlled = AgentInfo->IsLocallyControlled();
	const bool bIsOwnerActorAuthoritative = AgentInfo->IsOwnerActorAuthoritative();
	const EGunnerActionNetMethod ActionNetMethod = ActionDefinition->ActionCDO->GetActionNetMethod();
	const bool bIsRemoteTriggerable = ActionDefinition->ActionCDO->IsRemoteTriggerable();


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

		GR_LOG_SUB(LogGunner, Error, TEXT("해당 호스트에서는 Action을(를) 실행할 수 없습니다."));
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
			NetPredictionHandle.GenerateNewHandle();
			LocalTriggerAction(ActionDefinition, NetPredictionHandle);
			TArray<FGunnerLocalActionTriggerState> LocalActionTriggerStates;
			AggregateActionTriggerStates(LocalActionTriggerStates);
			ServerTryTriggerAction(ActionDefinitionHandle, EventMessage, LocalActionTriggerStates, NetPredictionHandle);
			return;
		}

		if (bIsRemoteTriggerable)
		{
			ServerRemoteRequestTryTriggerAction(ActionDefinitionHandle, EventMessage);
			return;
		}

		GR_LOG_SUB(LogGunner, Error, TEXT("해당 호스트에서는 Action을(를) 실행할 수 없습니다."));
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
		FGunnerActionScopedNetPrediction ScopedNetPrediction(*this, AgentInfo->IsOwnerActorAuthoritative(), NewPredictionHandle);
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

	FGunnerActionScopedNetPrediction ScopedNetPrediction(*this, AgentInfo->IsOwnerActorAuthoritative(), NetSyncPointDelegates[Index].NewPredictionHandle);
	Delegate.ExecuteIfBound();
	NetSyncPointDelegates.RemoveAt(Index);
}

void UGunnerActionComponent::IncrementActionListLock()
{
	ActionScopeLockCount++;
}

void UGunnerActionComponent::DecrementActionListLock()
{
	ActionScopeLockCount--;
	if (ActionScopeLockCount == 0 && (ActionPendingAdds.IsEmpty() || ActionPendingRemoves.IsEmpty()))
	{
		TArray<FGunnerActionDefinition> PendingAdds = MoveTemp(ActionPendingAdds);
		TArray<FGunnerActionDefinitionHandle> PendingRemoves = MoveTemp(ActionPendingRemoves);

		for (const auto& ActionDefinition : PendingAdds)
		{
			AuthAddAction(ActionDefinition);
		}

		for (const auto& ActionDefinition : PendingRemoves)
		{
			AuthRemoveAction(ActionDefinition);
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
		return AgentInfo->IsOwnerActorAuthoritative() || Action->IsRemoteTriggerable();
	}

	checkNoEntry();
	return false;
}

FGunnerActionSideEffectDefinition UGunnerActionComponent::MakeSideEffectDefinition(TSubclassOf<UGunnerActionSideEffect> SideEffectClass)
{
	return FGunnerActionSideEffectDefinition(SideEffectClass);
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
		ActionComponent->TriggerSideEffect(MakeSideEffectDefinition(SideEffectClass), Action);
	}
}

void UGunnerActionComponent::BP_TriggerSideEffectToActorWithSideEffectDefinition(UGunnerAction* Action, AActor* SideEffectTarget, const FGunnerActionSideEffectDefinition& SideEffectDefinition)
{
	check(Action);
	if (!SideEffectTarget)
	{
		return;
	}

	if (UGunnerActionComponent* ActionComponent = GetActionComponentFromActor(SideEffectTarget))
	{
		ActionComponent->TriggerSideEffect(SideEffectDefinition, Action);
	}
}

void UGunnerActionComponent::TriggerSideEffect(const FGunnerActionSideEffectDefinition& NewSideEffectDefinition, UGunnerAction* Action)
{
	check(Action);
	SideEffectDefinitionArray.Add(NewSideEffectDefinition, NetPredictionHandle, AgentInfo->IsOwnerActorAuthoritative());
	if (!AgentInfo->IsOwnerActorAuthoritative() && Action->GetActionNetMethod() == EGunnerActionNetMethod::LocalPredicted && !NetPredictionHandle.IsExpired()) // TODO: Expired조건문 위로 옮기기
	{
		GR_LOG_SUB(LogGunner, Display, TEXT("SideEffect Added [%s, %s]"), *NewSideEffectDefinition.SideEffectClass->GetName(), *NewSideEffectDefinition.Handle.ToString());

		FGunneractionNetPredictionEvents::FPredictionEvent& PredictionEvent = FGunneractionNetPredictionEvents::GetPredictionEvent(NetPredictionHandle);
		PredictionEvent.OnPredictionEnded.AddLambda([this, SideEffectDefinitionHandle = NewSideEffectDefinition.Handle]()
		{
			GR_LOG_SUB(LogGunner, Display, TEXT("PredictionEnded And Removed [%s]"), *SideEffectDefinitionHandle.ToString());
			SideEffectDefinitionArray.Remove(SideEffectDefinitionHandle);
		});

		PredictionEvent.OnPredictionFailed.AddLambda([this, SideEffectDefinitionHandle = NewSideEffectDefinition.Handle]()
		{
			GR_LOG_SUB(LogGunner, Display, TEXT("PredictionFailed And Removed [%s]"), *SideEffectDefinitionHandle.ToString());
			SideEffectDefinitionArray.Remove(SideEffectDefinitionHandle);
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

	if (AgentInfo->IsOwnerActorAuthoritative())
	{
		NetMulticastSignal(SignClass, SignalDataObject, NetPredictionHandle);
		return;
	}

	if (Action->GetActionNetMethod() != EGunnerActionNetMethod::ServerAuthoritative)
	{
		InternalSignal(SignClass, SignalDataObject);
	}
}

void UGunnerActionComponent::NetMulticastSignal_Implementation(TSubclassOf<UGunnerActionSign> SignClass, UObject* SignalDataObject, FGunnerActionNetPredictionHandle PredictionHandle)
{
	if (!AgentInfo->IsLocallyControlled() || AgentInfo->IsOwnerActorAuthoritative() || (AgentInfo->IsLocallyControlled() && !PredictionHandle.IsValid()))
	{
		InternalSignal(SignClass, SignalDataObject);
	}
}

void UGunnerActionComponent::InternalSignal(TSubclassOf<UGunnerActionSign> SignClass, UObject* SignalDataObject)
{
	GR_LOG_SUB(LogGunner, Display, TEXT("Signal [%s]"), *SignClass->GetName());
	UGunnerActionSign* Sign = SignClass.GetDefaultObject();
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


		for (const FGunnerActionProperty& Property : PropertyArray.Items)
		{
			DisplayDebugManager.DrawString(FString::Printf(TEXT("%s: StaticValue: %f, DynamicValue: %f"), *Property.Tag.ToString(), Property.StaticValue, Property.DynamicValue));
		}


		for (const FGunnerActionSideEffectDefinitionItem& Item : SideEffectDefinitionArray.Items)
		{
			DisplayDebugManager.DrawString(FString::Printf(TEXT("SideEffect: %s"), *Item.SideEffectDefinition.SideEffectClass->GetName()));
		}
	}
}

void UGunnerActionComponent::HandleTriggerableActionOnAdded(const FGunnerActionDefinition& NewActionDefinition)
{
	if (!HasActionTriggerAuthority(NewActionDefinition.ActionCDO))
	{
		return;
	}

	BindActionTriggerEvent(NewActionDefinition);
	if (NewActionDefinition.ActionCDO->ShouldTriggerOnAdded())
	{
		TryTriggerAction(NewActionDefinition.Handle, FGunnerEventMessage());
	}
}

void UGunnerActionComponent::HandleTriggerableActionOnRemoved(const FGunnerActionDefinition& ActionDefinition)
{
	if (HasActionTriggerAuthority(ActionDefinition.ActionCDO))
	{
		UnbindActionTriggerEvent(ActionDefinition);
	}
}

void UGunnerActionComponent::BindActionTriggerEvent(const FGunnerActionDefinition& NewActionDefinition)
{
	UGunnerAction* Action = NewActionDefinition.ActionCDO;

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

void UGunnerActionComponent::OnRep_ActionDefinitions(const TArray<FGunnerActionDefinition>& OldActionDefinitions)
{
	if (!AgentInfo->OwnerActor.IsValid() || !AgentInfo->AgentActor.IsValid())
	{
		return;
	}

	TArray<FGunnerActionDefinition> NewActionDefinitions;

	for (auto& ActionDefinition : ActionDefinitions)
	{
		if (!OldActionDefinitions.ContainsByPredicate([ActionDefinition](const FGunnerActionDefinition& OldActionDefinition)
		{
			return (ActionDefinition.Handle == OldActionDefinition.Handle);
		}))
		{
			ActionDefinition.ActionInstance = NewGunnerAction<UGunnerAction>(GetOwner(), ActionDefinition.ActionClass, ActionDefinition.Handle, AgentInfo);
			NewActionDefinitions.Add(ActionDefinition);
		}
	}

	for (auto& ActionDefinition : ActionDefinitions)
	{
		if (ActionDefinition.ActionInstance.GetClass() != ActionDefinition.ActionClass)
		{
			ActionDefinition.ActionInstance = NewGunnerAction<UGunnerAction>(GetOwner(), ActionDefinition.ActionClass, ActionDefinition.Handle, AgentInfo);
		}
	}


	for (const auto& NewActionDefinition : NewActionDefinitions)
	{
		HandleTriggerableActionOnAdded(NewActionDefinition);
	}

	TArray<FGunnerActionDefinition> RemoveActionDefinitions;
	for (const auto& OldActionDefinition : OldActionDefinitions)
	{
		if (!ActionDefinitions.ContainsByPredicate([OldActionDefinition](const FGunnerActionDefinition& ActionDefinition)
		{
			return (ActionDefinition.Handle == OldActionDefinition.Handle);
		}))
		{
			RemoveActionDefinitions.Add(OldActionDefinition);
		}
	}

	for (const auto& RemoveActionDefinition : RemoveActionDefinitions)
	{
		HandleTriggerableActionOnRemoved(RemoveActionDefinition);
	}
}

void UGunnerActionComponent::OnActionEnded(FGunnerActionDefinitionHandle ActionDefinitionHandle, UGunnerAction* Action)
{
	OwnedTags.RemoveTags(Action->GetActionOwnedTags());

	FGunnerActionDefinition* ActionDefinition = FindActionDefinitionByHandle(ActionDefinitionHandle);
	if (ActionDefinition && ActionDefinition->ActionCDO->GetActionNetMethod() == EGunnerActionNetMethod::LocalPredicted)
	{
		TArray<FGunnerActionDefinitionHandle> HandlesToRemove;
		for (const auto& [Handle, EventMessage, TriggerTime] : NetTriggerDelayedActions)
		{
			if (TriggerTime - GetWorld()->GetTimeSeconds() >= 0.2)
			{
				// 부정행위 또는 동기화 오류
				HandlesToRemove.Add(Handle);
				UE_DEBUG_BREAK();
				continue;
			}

			if (!Handle.IsValid())
			{
				continue;
			}

			FGunnerActionDefinition* DelayedActionDefinition = FindActionDefinitionByHandle(Handle);
			if (!DelayedActionDefinition)
			{
				continue;
			}

			if (!CanTriggerAction(*DelayedActionDefinition, EventMessage))
			{
				continue;
			}

			HandlesToRemove.Add(Handle);
			LocalTriggerAction(DelayedActionDefinition);
		}

		for (const auto& Handle : HandlesToRemove)
		{
			NetTriggerDelayedActions.RemoveAll([Handle](const FGunnerNetTriggerDelayedAction& TriggerDelayedAction)
			{
				return TriggerDelayedAction.ActionDefinitionHandle == Handle;
			});
		}
	}
}

FGunnerActionDefinition* UGunnerActionComponent::FindActionDefinitionByHandle(FGunnerActionDefinitionHandle ActionDefinitionHandle)
{
	check(ActionDefinitionHandle.IsValid());

	FGunnerActionDefinition* FoundedInActionDefinitions = ActionDefinitions.FindByPredicate([ActionDefinitionHandle](const FGunnerActionDefinition& ActionDefinition)
	{
		return ActionDefinition.Handle == ActionDefinitionHandle;
	});

	// If not found in ActionDefinitions, try to find in ActionPendingAdds
	return FoundedInActionDefinitions
		       ? FoundedInActionDefinitions
		       : ActionPendingAdds.FindByPredicate([ActionDefinitionHandle](const FGunnerActionDefinition& ActionDefinition)
		       {
			       return ActionDefinition.Handle == ActionDefinitionHandle;
		       });
}

bool UGunnerActionComponent::CanTriggerAction(const FGunnerActionDefinition& ActionDefinition, const FGunnerEventMessage& EventMessage) const
{
	ActionDefinition.ActionInstance->SetActionCurrentEventMessage(EventMessage);
	return ActionDefinition.ActionInstance->OnCanTriggerAction()
		&& OwnedTags.HasAll(ActionDefinition.ActionCDO->GetShouldHaveTags())
		&& !OwnedTags.HasAny(ActionDefinition.ActionCDO->GetShouldNotHaveTags());
}

void UGunnerActionComponent::LocalTriggerAction(FGunnerActionDefinition* ActionDefinition, FGunnerActionNetPredictionHandle PredictionHandle /*= FGunnerActionNetPredictionHandle()*/)
{
	check(ActionDefinition->ActionInstance);
	OwnedTags.AppendTags(ActionDefinition->ActionInstance->GetActionOwnedTags());
	ActionDefinition->ActionTriggerID++;


	ActionDefinition->ActionInstance->InitPredictionHandle = PredictionHandle;
	FGunnerActionScopedNetPrediction ScopedNetPrediction(*this, AgentInfo->IsOwnerActorAuthoritative(), PredictionHandle);
	ActionDefinition->ActionInstance->OnTriggerAction();
}


void UGunnerActionComponent::ClientTriggerActionRequestFailed_Implementation(FGunnerActionDefinitionHandle ActionDefinitionHandle, FGunnerActionNetPredictionHandle PredictionHandle)
{
	FGunnerActionDefinition* ActionDefinition = FindActionDefinitionByHandle(ActionDefinitionHandle);
	check(ActionDefinition);
	ActionDefinition->ActionInstance->OnEndAction();

	GR_LOG_SUB(LogGunner, Display, TEXT("PredictionHandle [%s]"), *PredictionHandle.ToString());
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

void UGunnerActionComponent::AggregateActionTriggerStates(TArray<FGunnerLocalActionTriggerState>& OutActionTriggerStates)
{
	for (const FGunnerActionDefinition& ActionDefinition : ActionDefinitions)
	{
		FGunnerLocalActionTriggerState ActionTriggerState = {
			.ActionDefinitionHandle = ActionDefinition.Handle,
			.ActionTriggerID = ActionDefinition.ActionTriggerID,
			.bIsTriggering = ActionDefinition.ActionInstance->IsTriggering()
		};

		OutActionTriggerStates.Add(ActionTriggerState);
	}
}

void UGunnerActionComponent::AuthAddProperty(FGameplayTag Tag, float Value)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	FGunnerActionProperty NewProperty;
	NewProperty.Tag = Tag;
	NewProperty.StaticValue = Value;
	NewProperty.MarkPropertyDirty();
	PropertyArray.AuthAdd(NewProperty);
}

void UGunnerActionComponent::AuthRemoveProperty(FGameplayTag Tag)
{
	if (GetOwner()->HasAuthority())
	{
		PropertyArray.AuthRemove(Tag);
	}
}

void UGunnerActionComponent::AuthRemoveAllProperties()
{
	if (GetOwner()->HasAuthority())
	{
		PropertyArray.AuthRemoveAll();
	}
}


FGunnerActionProperty* UGunnerActionComponent::GetProperty2(FGameplayTag Tag)
{
	for (FGunnerActionProperty& Property : PropertyArray.Items)
	{
		if (Property.Tag == Tag)
		{
			return &Property;
		}
	}
	return nullptr;
}

void UGunnerActionComponent::OnAdded(const FGunnerActionSideEffectDefinition& SideEffectDefinition, FGunnerActionNetPredictionHandle PredictionHandle)
{
	PropertyArray.OnSideEffectDefinitionAdded(SideEffectDefinition, PredictionHandle.IsValid() && !AgentInfo->IsOwnerActorAuthoritative());
}

void UGunnerActionComponent::OnRemoved(FGunnerActionSideEffectDefinitionHandle SideEffectDefinitionHandle)
{
	PropertyArray.OnSideEffectDefinitionRemoved(SideEffectDefinitionHandle);
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

	FGunnerActionProperty* PropertyPtr = ActionComponent->GetProperty2(Tag);
	if (!PropertyPtr)
	{
		return 0.0f;
	}

	return PropertyPtr->DynamicValue;
}

void UGunnerActionComponent::ClientTriggerAction_Implementation(FGunnerActionDefinitionHandle ActionDefinitionHandle, const FGunnerEventMessageReplicated& EventMessageReplicated)
{
	check(ActionDefinitionHandle.IsValid());
	FGunnerActionDefinition* ActionDefinition = FindActionDefinitionByHandle(ActionDefinitionHandle);
	check(ActionDefinition)
	ActionDefinition->ActionInstance->SetActionCurrentEventMessage(EventMessageReplicated.ToEventMessage());
	LocalTriggerAction(ActionDefinition);
}

void UGunnerActionComponent::ServerTryTriggerAction_Implementation(FGunnerActionDefinitionHandle ActionDefinitionHandle, const FGunnerEventMessageReplicated& EventMessageReplicated, const TArray<FGunnerLocalActionTriggerState>& ClientActionTriggerStates, FGunnerActionNetPredictionHandle PredictionHandle)
{
	GR_LOG_SUB(LogGunner, Display, TEXT("PredictionHandle [%s]"), *PredictionHandle.ToString());
	if (!ActionDefinitionHandle.IsValid())
	{
		UE_DEBUG_BREAK(); // FAIL DESYNC
	}
	FGunnerActionDefinition* ActionDefinition = FindActionDefinitionByHandle(ActionDefinitionHandle);
	if (!ActionDefinition)
	{
		UE_DEBUG_BREAK(); // FAIL DESYNC
	}

	LocalTriggerAction(ActionDefinition, PredictionHandle);
	return;

	if (CanTriggerAction(*ActionDefinition, EventMessageReplicated.ToEventMessage()))
	{
		LocalTriggerAction(ActionDefinition, PredictionHandle);
		return;
	}
	else
	{
		ClientTriggerActionRequestFailed(ActionDefinitionHandle, PredictionHandle);
		return;
	}


	for (const FGunnerLocalActionTriggerState& ClientActionTriggerState : ClientActionTriggerStates)
	{
		if (FGunnerActionDefinition* ServerActionDefinition = FindActionDefinitionByHandle(ClientActionTriggerState.ActionDefinitionHandle))
		{
			FGunnerLocalActionTriggerState ServerActionTriggerState = {
				.ActionDefinitionHandle = ServerActionDefinition->Handle,
				.ActionTriggerID = ServerActionDefinition->ActionTriggerID,
				.bIsTriggering = ServerActionDefinition->ActionInstance->IsTriggering()
			};

			if (ServerActionTriggerState == ClientActionTriggerState)
			{
				continue;
			}

			if (ServerActionTriggerState.ActionTriggerID > ClientActionTriggerState.ActionTriggerID)
			{
				UE_DEBUG_BREAK(); // TODO: 과거 보정 처리
			}

			NetTriggerDelayedActions.Add({ActionDefinitionHandle, EventMessageReplicated.ToEventMessage(), GetWorld()->GetTimeSeconds()});
			return;
		}
	}
	UE_DEBUG_BREAK();
}
