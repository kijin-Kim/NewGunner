// Fill out your copyright notice in the Description page of Project Settings.


#include "Event/NexusEventBindHelperInterface.h"

#include "Action/NexusActionComponent.h"


void INexusEventBindHelperInterface::BindEvents()
{
	BoundedEventCallbackHandles = SetupEvents();
}

void INexusEventBindHelperInterface::UnbindEvents(UNexusActionComponent* ActionComponent)
{
	if (ActionComponent)
	{
		for (FNexusEventCallbackHandle Handle : BoundedEventCallbackHandles)
		{
			ActionComponent->UnbindEventCallback(Handle);
		}
		BoundedEventCallbackHandles.Empty();
	}
}
