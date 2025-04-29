// Fill out your copyright notice in the Description page of Project Settings.


#include "NexusCheatManager.h"

#include "Action/NexusActionComponent.h"
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
		NX_LOG_SUB(ActionComponent->GetAgentActor(), LogNexusAction, Log, TEXT("액션 덤프: "));
		for (int i = 0; i < ActionDefs.Items.Num(); i++)
		{
			NX_LOG_SUB(ActionComponent->GetAgentActor(), LogNexusAction, Log, TEXT("    %d: %s"), i, *ActionDefs.Items[i].ToString());
		}
	}
}
