// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraControlComponent.h"

#include "GunnerCharacter.h"
#include "Gunner/_Core/Input/GunnerEventMessage.h"


UCameraControllerComponent::UCameraControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void UCameraControllerComponent::InitializeComponent()
{
	Super::InitializeComponent();
	if (AGunnerCharacter* GunnerCharacterOwner = GetOwner<AGunnerCharacter>())
	{
		GunnerCharacterOwner->OnPlayerStateChangedDelegate.AddUObject(this, &ThisClass::OnPlayerStateChanged);
	}
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

void UCameraControllerComponent::OnPlayerStateChanged(APlayerState* OldPlayerState, APlayerState* NewPlayerState)
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
		PawnOwner->AddControllerYawInput(LookAxisVector.X * BaseTurnRate * MouseSensitivity);
		PawnOwner->AddControllerPitchInput(LookAxisVector.Y * BaseTurnRate * MouseSensitivity);
	}
}
