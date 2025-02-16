// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraControlComponent.h"

#include "NexusEventMessage.h"
#include "Kismet/GameplayStatics.h"


UCameraControllerComponent::UCameraControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

TArray<FNexusEventCallbackHandle> UCameraControllerComponent::SetupEvents()
{
	if (UNexusEventManagerComponent* EventManagerComponent = UNexusEventManagerComponent::GetEventManagerComponentFromActor(GetOwner()))
	{
		return {
			EventManagerComponent->BindEventCallback<FNexusEventMessage>(FGameplayTag::RequestGameplayTag(FName(TEXT("Input.Look"))), this, &ThisClass::Look)
		};
	}
	return {};
}

void UCameraControllerComponent::InitCameraController()
{
	APawn* PawnOwner = GetOwner<APawn>();
	if (!PawnOwner->IsLocallyControlled())
	{
		return;
	}

	UnbindEvents(UNexusEventManagerComponent::GetEventManagerComponentFromActor(GetOwner()));
	BindEvents();
}

void UCameraControllerComponent::Look(FGameplayTag GameplayTag, const FNexusEventMessage& EventMessage)
{
	APawn* PawnOwner = GetOwner<APawn>();
	const FVector2D LookAxisVector = EventMessage.InputActionValue.Get<FVector2D>();
	if (PawnOwner->GetController())
	{
		float GlobalTimeDilation = UGameplayStatics::GetGlobalTimeDilation(GetWorld());
		PawnOwner->AddControllerYawInput(LookAxisVector.X * BaseTurnRate * MouseSensitivity * GlobalTimeDilation);
		PawnOwner->AddControllerPitchInput(LookAxisVector.Y * BaseTurnRate * MouseSensitivity * GlobalTimeDilation);
	}
}
