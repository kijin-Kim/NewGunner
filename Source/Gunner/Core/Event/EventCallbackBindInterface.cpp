// Fill out your copyright notice in the Description page of Project Settings.


#include "EventCallbackBindInterface.h"


// Add default functionality here for any IEventCallbackBindInterface functions that are not pure virtual.
void IEventCallbackBindInterface::BindEvents()
{
	BoundedEventCallbackHandles = SetupEvents();
}

void IEventCallbackBindInterface::UnbindEvents()
{
	for(FEventCallbackHandle Handle : BoundedEventCallbackHandles)
	{
		if (UEventManagerComponent* EventManagerComponent = GetEventManagerComponent())
		{
			EventManagerComponent->UnbindEventCallback(Handle);
		}
	}
	BoundedEventCallbackHandles.Empty();
}



