// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionComponent.h"

#include "NexusPropertyComponent.h"
#include "Action/NexusAction.h"
#include "Gunner/Action/GunnerActionSet.h"

void UGunnerActionComponent::OnSetupActionComponent()
{
	Super::OnSetupActionComponent();

	if (!IsOwnerActorAuthoritative())
	{
		return;
	}

	for (UGunnerActionSet* ActionSet : ActionSets)
	{
		for (const auto& [Tag, Value] : ActionSet->InitialProperties)
		{
			GetPropertyComponent()->AuthAddProperty(Tag, Value);
		}
		for (TSubclassOf<UNexusAction> ActionClass : ActionSet->InitialActionClasses)
		{
			if (ActionClass)
			{
				FNexusActionDef ActionDef(GetAgentActor(), ActionClass);
				AuthAddAction(ActionDef);
			}
		}
	}
}
