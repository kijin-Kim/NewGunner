// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionComponent.h"
#include "GunnerAction.h"
#include "Engine/Canvas.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerState.h"
#include "Gunner/Gunner.h"
#include "Gunner/Core/Event/EventManagerComponent.h"
#include "Gunner/Core/Input/GunnerEventMessage.h"
#include "Net/UnrealNetwork.h"

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

void UGunnerActionComponent::InitActionComponent(AActor* InOwnerActor, AActor* InAgentActor)
{
	AgentInfo->Init(InOwnerActor, InAgentActor);

	if (!AgentInfo->IsOwnerActorAuthoritative() && AgentInfo->IsLocallyControlled())
	{
		for (const auto& ActionDefinition : ActionDefinitions)
		{
			BindActionTriggerEvent(ActionDefinition);
		}
	}
}

void UGunnerActionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UGunnerActionComponent, ActionDefinitions, COND_OwnerOnly);
}

void UGunnerActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

FGunnerActionDefinitionHandle UGunnerActionComponent::AddAction(const FGunnerActionDefinition& ActionDefinition)
{
	if (!AgentInfo->IsOwnerActorAuthoritative())
	{
		return FGunnerActionDefinitionHandle();
	}

	BindActionTriggerEvent(ActionDefinition);
	if (ActionScopeLockCount > 0)
	{
		ActionPendingAdds.Add(ActionDefinition);
		return ActionDefinition.Handle;
	}

	return ActionDefinitions[ActionDefinitions.Add(ActionDefinition)].Handle;
}

void UGunnerActionComponent::RemoveAction(const FGunnerActionDefinitionHandle& ActionDefinitionHandle)
{
	if (!AgentInfo->IsOwnerActorAuthoritative())
	{
		return;
	}

	if (ActionScopeLockCount > 0)
	{
		ActionPendingRemoves.Add(ActionDefinitionHandle);
		return;
	}

	ActionDefinitions.RemoveAll([ActionDefinitionHandle](const FGunnerActionDefinition& ActionDefinition)
	{
		return ActionDefinition.Handle == ActionDefinitionHandle;
	});
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


	if (!CanTriggerAction(*ActionDefinition))
	{
		GR_LOG_SUB(LogGunner, Error, TEXT("Action [%s]을(를) 실행이 거부되었습니다"), *ActionDefinition->ActionClass->GetName());
		return;
	}


	if (AgentInfo->OwnerActor->GetNetMode() == NM_Standalone)
	{
		LocalTriggerAction(ActionDefinition, ActionDefinitionHandle, EventMessage);
		return;
	}

	const bool bIsLocallyControlled = AgentInfo->IsLocallyControlled();
	const bool bIsOwnerActorAuthoritative = AgentInfo->IsOwnerActorAuthoritative();
	const EGunnerActionNetMethod ActionNetMethod = ActionDefinition->ActionCDO->GetActionNetMethod();


	if (bIsOwnerActorAuthoritative)
	{
		if (bIsLocallyControlled || ActionNetMethod == EGunnerActionNetMethod::ServerOnly)
		{
			LocalTriggerAction(ActionDefinition, ActionDefinitionHandle, EventMessage);
			return;
		}

		if (ActionNetMethod == EGunnerActionNetMethod::ServerAuthoritative)
		{
			LocalTriggerAction(ActionDefinition, ActionDefinitionHandle, EventMessage);
			ClientTriggerAction(ActionDefinitionHandle, EventMessage);
			return;
		}
		GR_LOG_SUB(LogGunner, Error, TEXT("해당 호스트에서는 Action을(를) 실행할 수 없습니다."));
		return;
	}

	if (bIsLocallyControlled) // Autonomous Proxy
	{
		if (ActionNetMethod == EGunnerActionNetMethod::LocalOnly)
		{
			LocalTriggerAction(ActionDefinition, ActionDefinitionHandle, EventMessage);
			return;
		}

		if (ActionNetMethod == EGunnerActionNetMethod::LocalPredicted)
		{
			LocalTriggerAction(ActionDefinition, ActionDefinitionHandle, EventMessage);
			ServerTryTriggerAction(ActionDefinitionHandle, EventMessage);
			return;
		}
		GR_LOG_SUB(LogGunner, Error, TEXT("해당 호스트에서는 Action을(를) 실행할 수 없습니다."));
	}
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
		for (const auto& ActionDefinition : ActionPendingAdds)
		{
			AddAction(ActionDefinition);
		}
		ActionPendingAdds.Empty();

		for (const auto& ActionDefinitionHandle : ActionPendingRemoves)
		{
			RemoveAction(ActionDefinitionHandle);
		}
		ActionPendingRemoves.Empty();
	}
}

UGunnerActionComponent* UGunnerActionComponent::GetActionComponentFromActor(AActor* Actor)
{
	// ActionComponent는 Actor또는 PlayerState에 존재할 것을 가정합니다.
	if (UGunnerActionComponent* ActionComponent = Actor->GetComponentByClass<UGunnerActionComponent>())
	{
		return ActionComponent;
	}

	APawn* Pawn = Cast<APawn>(Actor);
	if (!Pawn)
	{
		return nullptr;
	}

	APlayerState* PlayerState = Pawn->GetPlayerState();
	if (!PlayerState)
	{
		return nullptr;
	}

	return PlayerState->GetComponentByClass<UGunnerActionComponent>();
}

void UGunnerActionComponent::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Arg)
{
	AActor* DebugTarget = HUD->GetCurrentDebugTargetActor();
	if (!DebugTarget)
	{
		return;
	}

	if (UGunnerActionComponent* ActionComponent = GetActionComponentFromActor(DebugTarget))
	{
		ActionComponent->InternalOnShowDebugInfo(DebugTarget, HUD, Canvas, DebugDisplayInfo, X, Arg);
	}
}

void UGunnerActionComponent::InternalOnShowDebugInfo(AActor* DebugTarget, AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Arg)
{
	if (!AgentInfo->AgentActor.IsValid() || DebugTarget != AgentInfo->AgentActor.Get())
	{
		return;
	}


	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	if (HUD->ShouldDisplayDebug(TEXT("ActionSystem")))
	{
		DisplayDebugManager.SetFont(GEngine->GetMediumFont());
		DisplayDebugManager.SetDrawColor(FColor::Orange);
		for (FGameplayTag Tag : OwnedTags)
		{
			DisplayDebugManager.DrawString(FString::Printf(TEXT("OwnedTags: %s"), *Tag.ToString()));
		}
	}
}

void UGunnerActionComponent::BindActionTriggerEvent(const FGunnerActionDefinition& NewActionDefinition)
{
	EGunnerActionNetMethod ActionNetMethod = NewActionDefinition.ActionCDO->GetActionNetMethod();
	bool bIsAutonmousProxy = !AgentInfo->IsOwnerActorAuthoritative() && AgentInfo->IsLocallyControlled();
	if (bIsAutonmousProxy && (ActionNetMethod != EGunnerActionNetMethod::LocalOnly && ActionNetMethod != EGunnerActionNetMethod::LocalPredicted))
	{
		return;
	}

	UGunnerAction* Action = NewActionDefinition.ActionCDO;
	FGameplayTagContainer ActionTriggerEventTags = Action->GetActionTriggerEventTags();
	UEventManagerComponent* EventManagerComponent = AgentInfo->OwnerActor->GetComponentByClass<UEventManagerComponent>();
	if (!EventManagerComponent)
	{
		return;
	}

	for (FGameplayTag Tag : ActionTriggerEventTags)
	{
		EventManagerComponent->BindEventCallback<FGunnerEventMessage>(Tag, this, &ThisClass::OnActionEventTriggered, NewActionDefinition.Handle);
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
	for (const auto& ActionDefinition : ActionDefinitions)
	{
		if (!OldActionDefinitions.ContainsByPredicate([ActionDefinition](const FGunnerActionDefinition& OldActionDefinition)
		{
			return (ActionDefinition.Handle == OldActionDefinition.Handle);
		}))
		{
			NewActionDefinitions.Add(ActionDefinition);
		}
	}
	for (const auto& NewActionDefinition : NewActionDefinitions)
	{
		BindActionTriggerEvent(NewActionDefinition);
	}
}

void UGunnerActionComponent::OnActionEnded(FGunnerActionDefinitionHandle ActionDefinitionHandle, UGunnerAction* Action)
{
	check(Action && ActionDefinitionHandle.IsValid());
	FGunnerActionDefinition* ActionDefinition = FindActionDefinitionByHandle(ActionDefinitionHandle);
	check(ActionDefinition);
	ActionDefinition->ActionInstances.Remove(Action);
	OwnedTags.RemoveTags(Action->GetActionOwnedTags());
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

bool UGunnerActionComponent::CanTriggerAction(const FGunnerActionDefinition& ActionDefinition) const
{
	return ActionDefinition.ActionCDO->CanTriggerAction()
		&& OwnedTags.HasAll(ActionDefinition.ActionCDO->GetShouldHaveTags())
		&& !OwnedTags.HasAny(ActionDefinition.ActionCDO->GetShouldNotHaveTags());
}

void UGunnerActionComponent::LocalTriggerAction(FGunnerActionDefinition* ActionDefinition, FGunnerActionDefinitionHandle ActionDefinitionHandle, const FGunnerEventMessage& EventMessage)
{
	if (EventMessage.GetInstigator())
	{
		GR_LOG_SUB(LogGunner, Display, TEXT("Instiagtor: [%s]"), *EventMessage.GetInstigator()->GetName());
	}
	if (EventMessage.GetTargetActor())
	{
		GR_LOG_SUB(LogGunner, Display, TEXT("Targetactor: [%s]"), *EventMessage.GetTargetActor()->GetName());
	}
	if (EventMessage.GetEventDataObject())
	{
		GR_LOG_SUB(LogGunner, Display, TEXT("EventDataObject: [%s]"), *EventMessage.GetEventDataObject()->GetName());
	}

	GR_LOG_SUB(LogGunner, Display, TEXT("InputActionValue: [%s]"), *EventMessage.GetInputActionValue().ToString());


	UGunnerAction* NewAction = NewObject<UGunnerAction>(GetOwner(), ActionDefinition->ActionClass);
	check(NewAction);
	ActionDefinition->ActionInstances.Add(NewAction);
	NewAction->OnGunnerActionEndedDelegate.BindUObject(this, &UGunnerActionComponent::OnActionEnded);
	OwnedTags.AppendTags(NewAction->GetActionOwnedTags());
	NewAction->TriggerAction(ActionDefinitionHandle, AgentInfo, EventMessage);
}

void UGunnerActionComponent::ClientTriggerAction_Implementation(FGunnerActionDefinitionHandle ActionDefinitionHandle, const FGunnerEventMessage& EventMessage)
{
	check(ActionDefinitionHandle.IsValid());
	FGunnerActionDefinition* ActionDefinition = FindActionDefinitionByHandle(ActionDefinitionHandle);
	check(ActionDefinition)
	LocalTriggerAction(ActionDefinition, ActionDefinitionHandle, EventMessage);
}

void UGunnerActionComponent::ServerTryTriggerAction_Implementation(FGunnerActionDefinitionHandle ActionDefinitionHandle, const FGunnerEventMessage& EventMessage)
{
	if (!ActionDefinitionHandle.IsValid())
	{
		UE_DEBUG_BREAK(); // FAIL DESYNC
	}
	FGunnerActionDefinition* ActionDefinition = FindActionDefinitionByHandle(ActionDefinitionHandle);
	if (!ActionDefinition)
	{
		UE_DEBUG_BREAK(); // FAIL DESYNC
	}

	if (!CanTriggerAction(*ActionDefinition))
	{
		UE_DEBUG_BREAK(); // FAIL DESYNC SHOULD ROLLBACK
	}

	LocalTriggerAction(ActionDefinition, ActionDefinitionHandle, EventMessage);
}
