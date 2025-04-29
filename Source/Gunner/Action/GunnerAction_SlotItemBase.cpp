// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerAction_SlotItemBase.h"

#include "Action/NexusActionComponent.h"



AActor* UGunnerAction_SlotItemBase::GetSourceObjectActor() const
{
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetAgentActor());
	check(ActionComponent);
	const FNexusActionDef* ActionDef = ActionComponent->FindActionDefByHandle(GetActionDefHandle());
	return Cast<AActor>(ActionDef->SourceObject);
}
