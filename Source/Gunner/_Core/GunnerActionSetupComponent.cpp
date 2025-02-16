// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionSetupComponent.h"

#include "GunnerActionSet.h"
#include "NexusActionComponent.h"
#include "Action/NexusAction.h"


// Sets default values for this component's properties
UGunnerActionSetupComponent::UGunnerActionSetupComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGunnerActionSetupComponent::AuthSetupActionSets()
{
	AActor* ActorOwner = GetOwner();
	check(ActorOwner);
	check(ActorOwner->HasAuthority());
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(ActorOwner);;
	check(ActionComponent);


	for (UGunnerActionSet* ActionSet : ActionSets)
	{
		for (const auto& [Tag, Value] : ActionSet->InitialProperties)
		{
			ActionComponent->AuthAddProperty(Tag, Value);
		}
		for (TSubclassOf<UNexusAction> ActionClass : ActionSet->InitialActionClasses)
		{
			if (ActionClass)
			{
				FNexusActionDef ActionDefinition(ActorOwner, ActionClass);
				ActionComponent->AuthAddAction(ActionDefinition);
			}
		}
	}
}
