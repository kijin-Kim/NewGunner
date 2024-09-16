// Fill out your copyright notice in the Description page of Project Settings.


#include "TestGunnerAction.h"

#include "GunnerActionAgentInfo.h"
#include "GunnerActionComponent.h"
#include "TestChildGunnerAction.h"

UTestGunnerAction::UTestGunnerAction()
{
	ActionNetMethod = EGunnerActionNetMethod::ServerAuthoritative;
}

void UTestGunnerAction::TriggerAction(FGunnerActionDefinitionHandle InActionDefinitionHandle, TWeakPtr<FGunnerActionAgentInfo> InActionAgentInfo)
{
	Super::TriggerAction(InActionDefinitionHandle, InActionAgentInfo);

	AActor* OwnerActor = AgentInfo.Pin()->OwnerActor.Get();
	if (OwnerActor && OwnerActor->HasAuthority())
	{
		FGunnerActionDefinition ActionDefinition(OwnerActor, UTestChildGunnerAction::StaticClass());
		OwnerActor->GetComponentByClass<UGunnerActionComponent>()->TryTriggerAction(OwnerActor->GetComponentByClass<UGunnerActionComponent>()->AddAction(ActionDefinition));
	}
}

void UTestGunnerAction::EndAction()
{
	Super::EndAction();
}
