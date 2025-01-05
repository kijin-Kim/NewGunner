// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerAction.h"
#include "Gunner/Gunner.h"


UWorld* UGunnerAction::GetWorld() const
{
	// https://forums.unrealengine.com/t/can-you-use-a-blueprint-function-library-in-an-object-class/350918/37
	if (HasAllFlags(RF_ClassDefaultObject))
	{
		return nullptr;
	}
	return GetOuter()->GetWorld();
}

void UGunnerAction::InitializeGunnerAction(FGunnerActionDefinitionHandle InActionDefinitionHandle, TWeakPtr<FGunnerActionAgentInfo> InAgentInfo)
{
	check(InActionDefinitionHandle.IsValid());
	check(InAgentInfo.IsValid());
	ActionDefinitionHandle = InActionDefinitionHandle;
	AgentInfo = InAgentInfo;
}

void UGunnerAction::SetActionCurrentEventMessage(const FGunnerEventMessage& InEventMessage)
{
	EventMessage = InEventMessage;
}

void UGunnerAction::OnActionAdded_Implementation()
{
	GR_LOG_SUB(LogGunner, Display, TEXT("[%s]"), *GetName());
}

bool UGunnerAction::OnCanTriggerAction_Implementation() const
{
	GR_LOG_SUB(LogGunner, Display, TEXT("[%s]"), *GetName());
	return (bIsRetriggerable || !bIsTriggering);
}

void UGunnerAction::OnTriggerAction_Implementation()
{
	GR_LOG_SUB(LogGunner, Display, TEXT("[%s]"), *GetName());
	check(ActionDefinitionHandle.IsValid());
	check(AgentInfo.IsValid());
	check(bIsRetriggerable || !bIsTriggering);


	if (bIsTriggering && bIsRetriggerable)
	{
		EndAction();
	}

	bIsTriggering = true;
}

void UGunnerAction::OnEndAction_Implementation()
{
	GR_LOG_SUB(LogGunner, Display, TEXT("[%s]"), *GetName());

	if (!bIsTriggering)
	{
		return;
	}
	bIsTriggering = false;
	check(ActionDefinitionHandle.IsValid());
	OnGunnerActionEndedDelegate.Broadcast(ActionDefinitionHandle, this);
}

void UGunnerAction::EndAction()
{
	if (bIsTriggering)
	{
		OnEndAction();
	}
}
