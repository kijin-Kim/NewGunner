// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionComponent.h"

#include "Action/NexusAction.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Gunner/Action/GunnerActionSet.h"
#include "Gunner/Action/GunnerAction_Fire.h"

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
			UGunnerAction_Fire::DrawDebugHitBoxData(GetWorld(), Entry.DebugHitBoxData, Entry.bHitConfirmed ? FColor::Green : FColor::Red, true, 0.0f);
			FVector Location = Entry.ServerLocation;
			Location.Z += Entry.ClientClaimedHitCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.0f + 20.0f;
			DrawDebugString(GetWorld(), Location, FString::Printf(TEXT("Server Rewounded TimeStamp=%f, FoundSnapshot=%s"), Entry.ServerRewoundedTimeStamp, Entry.bFoundSnapshot ? TEXT("true") : TEXT("false")), nullptr, Entry.bHitConfirmed ? FColor::Green : FColor::Red,  -1.0f, true);
		}
	}
}
