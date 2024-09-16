// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionComponent.h"
#include "GunnerAction.h"
#include "Gunner/Gunner.h"
#include "Net/UnrealNetwork.h"

UGunnerActionComponent::UGunnerActionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	AgentInfo = MakeShared<FGunnerActionAgentInfo>();
}

void UGunnerActionComponent::InitActionComponent(AActor* InOwnerActor, AActor* InAgentActor)
{
	AgentInfo->Init(InOwnerActor, InAgentActor);
	UGunnerAction::OnGunnerActionEndedDelegate.BindUObject(this, &UGunnerActionComponent::OnActionEnded);
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

void UGunnerActionComponent::TryTriggerAction(FGunnerActionDefinitionHandle ActionDefinitionHandle)
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


	if (AgentInfo->OwnerActor->GetNetMode() == NM_Standalone)
	{
		if (ActionDefinition->ActionCDO->CanTriggerAction())
		{
			LocalTriggerAction(ActionDefinition, ActionDefinitionHandle);
		}
		return;
	}

	const bool bIsLocallyControlled = AgentInfo->IsLocallyControlled();
	const bool bIsOwnerActorAuthoritative = AgentInfo->IsOwnerActorAuthoritative();
	const EGunnerActionNetMethod ActionNetMethod = ActionDefinition->ActionCDO->GetActionNetMethod();


	if (bIsLocallyControlled && !bIsOwnerActorAuthoritative)
	{
		if (ActionNetMethod == EGunnerActionNetMethod::ClientOnly)
		{
			if (ActionDefinition->ActionCDO->CanTriggerAction())
			{
				LocalTriggerAction(ActionDefinition, ActionDefinitionHandle);
			}
		}
		else if (ActionNetMethod == EGunnerActionNetMethod::ClientPredicted)
		{
			if (ActionDefinition->ActionCDO->CanTriggerAction())
			{
				LocalTriggerAction(ActionDefinition, ActionDefinitionHandle);
				ServerTryTriggerAction(ActionDefinitionHandle);
			}
		}
	}
	else if (bIsOwnerActorAuthoritative)
	{
		if (ActionNetMethod == EGunnerActionNetMethod::ServerOnly)
		{
			if (ActionDefinition->ActionCDO->CanTriggerAction())
			{
				LocalTriggerAction(ActionDefinition, ActionDefinitionHandle);
			}
		}
		else if (ActionNetMethod == EGunnerActionNetMethod::ServerAuthoritative)
		{
			if (ActionDefinition->ActionCDO->CanTriggerAction())
			{
				LocalTriggerAction(ActionDefinition, ActionDefinitionHandle);
				ClientTriggerAction(ActionDefinitionHandle);
			}
		}
	}
}

void UGunnerActionComponent::TEST_TRIGGER_ACTIONS()
{
	ACTION_LIST_SCOPE_LOCK();
	for (const auto& ActionDefinition : ActionDefinitions)
	{
		TryTriggerAction(ActionDefinition.Handle);
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

void UGunnerActionComponent::OnRep_ActionDefinitions()
{
	for (const auto& ActionDefinition : ActionDefinitions)
	{
		GR_LOG_SUB(LogGunner, Warning, TEXT("ActionClass: %s"), *GetNameSafe(ActionDefinition.ActionClass));
		GR_LOG_SUB(LogGunner, Warning, TEXT("SourceObject: %s"), *GetNameSafe(ActionDefinition.SourceObject.Get()));
		GR_LOG_SUB(LogGunner, Warning, TEXT("Handle: %s"), *ActionDefinition.Handle.ToString());
	}
}


void UGunnerActionComponent::OnActionEnded(FGunnerActionDefinitionHandle ActionDefinitionHandle, UGunnerAction* Action)
{
	check(Action && ActionDefinitionHandle.IsValid());
	FGunnerActionDefinition* ActionDefinition = FindActionDefinitionByHandle(ActionDefinitionHandle);
	check(ActionDefinition);

	ActionDefinition->ActionInstances.Remove(Action);
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

void UGunnerActionComponent::LocalTriggerAction(FGunnerActionDefinition* ActionDefinition, FGunnerActionDefinitionHandle ActionDefinitionHandle)
{
	UGunnerAction* NewAction = NewObject<UGunnerAction>(GetOwner(), ActionDefinition->ActionClass);
	check(NewAction);
	ActionDefinition->ActionInstances.Add(NewAction);
	NewAction->TriggerAction(ActionDefinitionHandle, AgentInfo);
}

void UGunnerActionComponent::ClientTriggerAction_Implementation(FGunnerActionDefinitionHandle ActionDefinitionHandle)
{
	check(ActionDefinitionHandle.IsValid());
	FGunnerActionDefinition* ActionDefinition = FindActionDefinitionByHandle(ActionDefinitionHandle);
	check(ActionDefinition)
	LocalTriggerAction(ActionDefinition, ActionDefinitionHandle);
}

void UGunnerActionComponent::ServerTryTriggerAction_Implementation(FGunnerActionDefinitionHandle ActionDefinitionHandle)
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

	if (!ActionDefinition->ActionCDO->CanTriggerAction())
	{
		UE_DEBUG_BREAK(); // FAIL
	}

	LocalTriggerAction(ActionDefinition, ActionDefinitionHandle);
}
