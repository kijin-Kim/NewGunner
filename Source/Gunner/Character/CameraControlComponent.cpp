// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraControlComponent.h"

#include "GunnerCharacter.h"
#include "Gunner/_Core/Input/GunnerEventMessage.h"
#include "Kismet/GameplayStatics.h"


UCameraControllerComponent::UCameraControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

TArray<FGunnerEventCallbackHandle> UCameraControllerComponent::SetupEvents()
{
	if (UGunnerEventManagerComponent* EventManagerComponent = UGunnerEventManagerComponent::GetEventManagerComponentFromActor(GetOwner()))
	{
		return {
			EventManagerComponent->BindEventCallback<FGunnerEventMessage>(FGameplayTag::RequestGameplayTag(FName(TEXT("Input.Look"))), this, &ThisClass::Look)
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

	UnbindEvents(UGunnerEventManagerComponent::GetEventManagerComponentFromActor(GetOwner()));
	BindEvents();
}

void UCameraControllerComponent::Look(FGameplayTag GameplayTag, const FGunnerEventMessage& EventMessage)
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
