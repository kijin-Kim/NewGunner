// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerEventCallbackBindInterface.h"


// Add default functionality here for any IGunnerEventCallbackBindInterface functions that are not pure virtual.
void IGunnerEventCallbackBindInterface::BindEvents()
{
	BoundedEventCallbackHandles = SetupEvents();
}

void IGunnerEventCallbackBindInterface::UnbindEvents()
{
	for (FGunnerEventCallbackHandle Handle : BoundedEventCallbackHandles)
	{
		if (UGunnerEventManagerComponent* EventManagerComponent = UGunnerEventManagerComponent::GetEventManagerComponentFromActor(Cast<AActor>(this)))
		{
			EventManagerComponent->UnbindEventCallback(Handle);
		}
	}
	BoundedEventCallbackHandles.Empty();
}
