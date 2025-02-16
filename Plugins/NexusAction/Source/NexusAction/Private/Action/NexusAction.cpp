// Fill out your copyright notice in the Description page of Project Settings.


#include "Action/NexusAction.h"


UWorld* UNexusAction::GetWorld() const
{
	// https://forums.unrealengine.com/t/can-you-use-a-blueprint-function-library-in-an-object-class/350918/37
	if (HasAllFlags(RF_ClassDefaultObject))
	{
		return nullptr;
	}
	return GetOuter()->GetWorld();
}

void UNexusAction::InitializeAction(FNexusActionDefHandle InActionDefinitionHandle, TWeakPtr<FNexusAgentInfo> InAgentInfo)
{
	check(InActionDefinitionHandle.IsValid());
	check(InAgentInfo.IsValid());
	ActionDefinitionHandle = InActionDefinitionHandle;
	AgentInfo = InAgentInfo;
}

void UNexusAction::SetActionCurrentEventMessage(const FNexusEventMessage& InEventMessage)
{
	EventMessage = InEventMessage;
}

void UNexusAction::OnActionAdded_Implementation()
{
}

bool UNexusAction::OnCanTriggerAction_Implementation() const
{
	return true;
}

void UNexusAction::OnTriggerAction_Implementation()
{
	check(ActionDefinitionHandle.IsValid());
	check(AgentInfo.IsValid());
	check(bIsRetriggerable || !bIsTriggering);


	if (bIsTriggering && bIsRetriggerable)
	{
		EndAction();
	}

	bIsTriggering = true;
}

void UNexusAction::OnEndAction_Implementation()
{
	if (!bIsTriggering)
	{
		return;
	}
	bIsTriggering = false;
	check(ActionDefinitionHandle.IsValid());
	OnActionEndedDelegate.Broadcast(ActionDefinitionHandle, this);
}

void UNexusAction::EndAction()
{
	if (bIsTriggering)
	{
		OnEndAction();
	}
}
