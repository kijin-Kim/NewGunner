// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionComponent.h"

#include "Action/NexusAction.h"
#include "Gunner/Action/GunnerActionSet.h"

UGunnerActionComponent::UGunnerActionComponent()
{
	SetIsReplicatedByDefault(true);
}

void UGunnerActionComponent::OnSetupActionComponent()
{
	Super::OnSetupActionComponent();
	for (UGunnerActionSet* ActionSet : ActionSets)
	{
		for (const auto& [Tag, Value] : ActionSet->InitialProperties)
		{
			AddProperty(Tag, Value);
		}

		if (!IsOwnerActorAuthoritative())
		{
			continue;
		}

		for (TSubclassOf<UNexusAction> ActionClass : ActionSet->InitialActionClasses)
		{
			if (ActionClass)
			{
				AuthAddAction(ActionClass, GetAgentActor());
			}
		}
	}
}
