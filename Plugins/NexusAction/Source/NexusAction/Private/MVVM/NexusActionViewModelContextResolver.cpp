// Fill out your copyright notice in the Description page of Project Settings.


#include "MVVM/NexusActionViewModelContextResolver.h"

#include "Action/NexusActionComponent.h"
#include "Blueprint/UserWidget.h"

UNexusActionComponent* UNexusActionViewModelContextResolver::GetActionComponent(const UUserWidget* UserWidget) const
{
	if (!UserWidget)
	{
		return nullptr;
	}

	APawn* Pawn = UserWidget->GetOwningPlayerPawn();
	if (Pawn)
	{
		return UNexusActionComponent::GetActionComponentFromActor(Pawn);
	}
	return nullptr;
}
