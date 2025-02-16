// Fill out your copyright notice in the Description page of Project Settings.


#include "Event/NexusEventInterface.h"

UNexusEventInterface::UNexusEventInterface(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


void INexusEventInterface::BindEvents()
{
	BoundedEventCallbackHandles = SetupEvents();
}

void INexusEventInterface::UnbindEvents(UNexusEventManagerComponent* EventManagerComponent)
{
	if (EventManagerComponent)
	{
		for (FNexusEventCallbackHandle Handle : BoundedEventCallbackHandles)
		{
			EventManagerComponent->UnbindEventCallback(Handle);
		}
		BoundedEventCallbackHandles.Empty();
	}
}
