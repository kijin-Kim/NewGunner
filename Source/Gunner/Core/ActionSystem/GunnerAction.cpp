// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerAction.h"

#include "Gunner/Gunner.h"

bool UGunnerAction::CanTriggerAction() const
{
	return CanTrigger();
}

void UGunnerAction::TriggerAction(FGunnerActionDefinitionHandle InActionDefinitionHandle, TWeakPtr<FGunnerActionAgentInfo> InAgentInfo, const FGunnerEventMessage& InEventMessage)
{
	GR_LOG_SUB(LogGunner, Display, TEXT("[%s]"), *GetName());
	check(InActionDefinitionHandle.IsValid());
	check(InAgentInfo.IsValid());
	check(!bIsRunning);
	ActionDefinitionHandle = InActionDefinitionHandle;
	AgentInfo = InAgentInfo;
	bIsRunning = true;
	EventMessage = InEventMessage;
	
	Trigger(InEventMessage);
}

void UGunnerAction::EndAction()
{
	GR_LOG_SUB(LogGunner, Display, TEXT("[%s]"), *GetName());
	check(bIsRunning);
	bIsRunning = false;
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



