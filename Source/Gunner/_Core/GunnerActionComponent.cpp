// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionComponent.h"

#include "Action/NexusAction.h"
#include "Gunner/Action/GunnerActionSet.h"
#include "Gunner/Action/GunnerAction_DebugFire.h"

UGunnerActionComponent::UGunnerActionComponent()
{
	SetIsReplicatedByDefault(true);
}

void UGunnerActionComponent::OnSetupActionComponent()
{
	Super::OnSetupActionComponent();
	for (UGunnerActionSet* ActionSet : ActionSets)
	{
		if (!IsOwnerActorAuthoritative())
		{
			continue;
		}

		for (const auto& [Tag, Value] : ActionSet->InitialProperties)
		{
			AuthAddProperty(Tag, Value);
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

void UGunnerActionComponent::ClientSendDebugHitConfirmedData_Implementation(const TArray<FGunnerDebugHitConfirmedDataEntry>& DebugHitConfirmedData)
{
	for (const FGunnerDebugHitConfirmedDataEntry& Entry : DebugHitConfirmedData)
	{
		if (Entry.ClientClaimedHitCharacter)
		{
			UGunnerAction_DebugFire::DrawDebugHitBoxData(GetWorld(), Entry.DebugHitBoxData, Entry.bHitConfirmed ? FColor::Green : FColor::Red, true, 0.0f);
		}
	}
}
