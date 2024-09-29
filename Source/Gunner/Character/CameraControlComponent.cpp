// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraControlComponent.h"
#include "Gunner/Core/Input/GunnerEventMessage.h"


UCameraControllerComponent::UCameraControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void UCameraControllerComponent::InitializeComponent()
{
	Super::InitializeComponent();
	if (APawn* PawnOwner = GetOwner<APawn>())
	{
		PawnOwner->ReceiveControllerChangedDelegate.AddDynamic(this, &ThisClass::OnControllerChanged);
	}
}

TArray<FEventCallbackHandle> UCameraControllerComponent::SetupEvents()
{
	if (UEventManagerComponent* EventManagerComponent = GetEventManagerComponent())
	{
		return {
			EventManagerComponent->BindEventCallback<FGunnerEventMessage>(FGameplayTag::RequestGameplayTag(FName(TEXT("Input.Look"))), this, &ThisClass::Look)
		};
	}
	return {};
}

UEventManagerComponent* UCameraControllerComponent::GetEventManagerComponent() const
{
	AActor* ActorOwner = GetOwner();
	return ActorOwner ? ActorOwner->GetComponentByClass<UEventManagerComponent>() : nullptr;
}


void UCameraControllerComponent::OnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController)
{
	if (NewController && OldController != NewController)
	{
		UnbindEvents();
		BindEvents();
	}
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
