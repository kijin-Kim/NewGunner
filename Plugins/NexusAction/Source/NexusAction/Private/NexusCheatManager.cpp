// Fill out your copyright notice in the Description page of Project Settings.


#include "NexusCheatManager.h"

#include "Action/NexusActionComponent.h"
#include "GameFramework/HUD.h"

void UNexusCheatManager::SendEventToSelf(const FString& EventTagString)
{
	APlayerController* PC = GetPlayerController();
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	APawn* Pawn = PC->GetPawn();
	if (!Pawn)
	{
		return;
	}


	FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(*EventTagString);
	if (EventTag.IsValid())
	{
		UNexusActionComponent::SendEventToActor<FNexusEventMessage>(EventTag, FNexusEventMessage{}, Pawn);
	}
}

void UNexusCheatManager::SendEventToTarget(const FString& EventTagString)
{
	APlayerController* PC = GetPlayerController();
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	AHUD* HUD = PC->GetHUD();
	if (!HUD)
	{
		return;
	}

	AActor* CurrentDebugTargetActor = HUD->GetCurrentDebugTargetActor();
	if (!CurrentDebugTargetActor)
	{
		return;
	}

	FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(*EventTagString);
	if (EventTag.IsValid())
	{
		UNexusActionComponent::SendEventToActor<FNexusEventMessage>(EventTag, FNexusEventMessage{}, CurrentDebugTargetActor);
	}
}
