// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerAction.h"
#include "Gunner/Gunner.h"

bool UGunnerAction::CanTriggerAction() const
{
	return (bIsRetriggerable || !bIsTriggering) && CanTrigger();
}

void UGunnerAction::TriggerAction(FGunnerActionDefinitionHandle InActionDefinitionHandle, TWeakPtr<FGunnerActionAgentInfo> InAgentInfo, const FGunnerEventMessage& InEventMessage)
{
	GR_LOG_SUB(LogGunner, Display, TEXT("[%s]"), *GetName());
	check(InActionDefinitionHandle.IsValid());
	check(InAgentInfo.IsValid());
	check(bIsRetriggerable || !bIsTriggering);
	ActionDefinitionHandle = InActionDefinitionHandle;
	AgentInfo = InAgentInfo;
	EventMessage = InEventMessage;

	if(bIsTriggering && bIsRetriggerable)
	{
		EndAction();
		TriggerAction(InActionDefinitionHandle, InAgentInfo, InEventMessage);
		return;
	}
	
	bIsTriggering = true;
	Trigger(InEventMessage);
}

void UGunnerAction::EndAction()
{
	GR_LOG_SUB(LogGunner, Display, TEXT("[%s]"), *GetName());
	check(bIsTriggering);
	bIsTriggering = false;
	check(ActionDefinitionHandle.IsValid());
	check(OnGunnerActionEndedDelegate.ExecuteIfBound(ActionDefinitionHandle, this));
	End();
}


bool UGunnerAction::CanTrigger_Implementation() const
{
	return true;
}

void UGunnerAction::Trigger_Implementation(const FGunnerEventMessage& InEventMessage)
{
}

void UGunnerAction::End_Implementation()
{
}
