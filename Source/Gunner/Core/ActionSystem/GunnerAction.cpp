// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerAction.h"

#include "Gunner/Gunner.h"

bool UGunnerAction::CanTriggerAction()
{
	return true;
}

void UGunnerAction::TriggerAction(FGunnerActionDefinitionHandle InActionDefinitionHandle)
{
	GR_LOG_SUB(LogGunner, Warning, TEXT("Triggered action: %s"), *GetName());
	check(InActionDefinitionHandle.IsValid());
	check(!bIsRunning);
	bIsRunning = true;
	ActionDefinitionHandle = InActionDefinitionHandle;
}

void UGunnerAction::EndAction()
{
	GR_LOG_SUB(LogGunner, Warning, TEXT("Ended action: %s"), *GetName());
	check(bIsRunning);
	bIsRunning = false;
	check(ActionDefinitionHandle.IsValid());
	check(OnGunnerActionEndedDelegate.ExecuteIfBound(ActionDefinitionHandle, this));
}

