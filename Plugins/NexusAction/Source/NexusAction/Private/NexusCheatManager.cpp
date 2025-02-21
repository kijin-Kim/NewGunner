// Fill out your copyright notice in the Description page of Project Settings.


#include "NexusCheatManager.h"

#include "NexusActionComponent.h"
#include "NexusLog.h"
#include "Action/NexusAction.h"

void UNexusCheatManager::DumpAddedActions()
{
	APlayerController* PlayerController = GetOuterAPlayerController();
	APawn* Pawn = PlayerController->GetPawn();
	if (!Pawn)
	{
		return;
	}

	if (UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(Pawn))
	{
		const FNexusActionDefContainer& ActionDefs = ActionComponent->GetActionDefs();
		UE_LOG(LogNexusAction, Warning, TEXT("Dumping Added Actions: "));
		for (int i = 0; i < ActionDefs.Items.Num(); i++)
		{
			UE_LOG(LogNexusAction, Warning, TEXT("    %d: %s"), i, *ActionDefs.Items[i].ToString());
		}
	}
}
