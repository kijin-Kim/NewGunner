// Fill out your copyright notice in the Description page of Project Settings.


#include "Action/NexusAction.h"
#include "Action/NexusActionDef.h"


UWorld* UNexusAction::GetWorld() const
{
	// https://forums.unrealengine.com/t/can-you-use-a-blueprint-function-library-in-an-object-class/350918/37
	if (HasAllFlags(RF_ClassDefaultObject))
	{
		return nullptr;
	}
	return GetOuter()->GetWorld();
}

void UNexusAction::InitializeAction(const FNexusActionDef& InActionDef, TWeakPtr<FNexusAgentInfo> InAgentInfo)
{
	check(InAgentInfo.IsValid());
	ActionDef = InActionDef;
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
	OnActionEndedDelegate.Broadcast(ActionDef.Handle, this);
}

void UNexusAction::EndAction()
{
	if (bIsTriggering)
	{
		OnEndAction();
	}
}


