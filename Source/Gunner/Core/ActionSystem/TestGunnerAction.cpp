// Fill out your copyright notice in the Description page of Project Settings.


#include "TestGunnerAction.h"

UTestGunnerAction::UTestGunnerAction()
{
	ActionNetMethod = EGunnerActionNetMethod::ServerAuthoritative;
}

void UTestGunnerAction::TriggerAction(FGunnerActionDefinitionHandle InActionDefinitionHandle)
{
	Super::TriggerAction(InActionDefinitionHandle);
}

void UTestGunnerAction::EndAction()
{
	Super::EndAction();
}
