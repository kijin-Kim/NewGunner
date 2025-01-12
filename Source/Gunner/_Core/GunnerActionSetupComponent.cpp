// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionSetupComponent.h"

#include "GunnerActionSet.h"
#include "ActionSystem/GunnerActionComponent.h"
#include "Gunner/_Core/ActionSystem/GunnerAction.h"


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
	UGunnerActionComponent* ActionComponent = UGunnerActionComponent::GetActionComponentFromActor(ActorOwner);;
	check(ActionComponent);


	for (UGunnerActionSet* ActionSet : ActionSets)
	{
		for (const auto& [Tag, Value] : ActionSet->InitialProperties)
		{
			ActionComponent->AuthAddProperty(Tag, Value);
		}
		for (TSubclassOf<UGunnerAction> ActionClass : ActionSet->InitialActionClasses)
		{
			if (ActionClass)
			{
				FGunnerActionDefinition ActionDefinition(ActorOwner, ActionClass);
				ActionComponent->AuthAddAction(ActionDefinition);
			}
		}
	}
}
