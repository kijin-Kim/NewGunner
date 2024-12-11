// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerInputEventDispatcherComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GunnerEventMessage.h"
#include "GunnerInputTagMappingData.h"
#include "GameFramework/PlayerState.h"
#include "Gunner/Gunner.h"
#include "Gunner/_Core/Event/GunnerEventManagerComponent.h"


UGunnerInputEventDispatcherComponent::UGunnerInputEventDispatcherComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void UGunnerInputEventDispatcherComponent::InitializeComponent()
{
	Super::InitializeComponent();
	PlayerController = Cast<APlayerController>(GetOwner());
	if (PlayerController && PlayerController->IsLocalController())
	{
		PlayerController->OnPossessedPawnChanged.AddUniqueDynamic(this, &ThisClass::SetupInputEvent);
	}
}

void UGunnerInputEventDispatcherComponent::OnInputEvent(const FInputActionValue& InputActionValue, UEnhancedInputComponent* InputComponent, FGameplayTag InputTag)
{
	check(InputComponent);
	if (APlayerState* PlayerState = PlayerController->GetPlayerState<APlayerState>())
	{
		const FGunnerEventMessage EventMessage(PlayerController, nullptr, InputActionValue, nullptr);
		UGunnerEventManagerComponent::SendEventToActor<FGunnerEventMessage>(InputTag, EventMessage, PlayerState);
	}
}

void UGunnerInputEventDispatcherComponent::SetupInputEvent(APawn* OldPawn, APawn* NewPawn)
{
	if (!InputTagMappingData || (!NewPawn && !OldPawn))
	{
		return;
	}
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if (!Subsystem)
	{
		return;
	}

	UEnhancedInputComponent* InputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent);
	if (!InputComponent)
	{
		return;
	}

	Subsystem->ClearAllMappings();
	InputComponent->ClearActionBindings();
	
	for (const auto& [IMC, Priority] : InputTagMappingData->InputContextAndPriorities)
	{
		Subsystem->AddMappingContext(IMC, Priority);
	}
	
	for (const auto& [InputAction, TriggerEventMappings] : InputTagMappingData->InputTagMappings)
	{
		for (const auto& [TriggerEvent, InputTag] : TriggerEventMappings)
		{
			InputComponent->BindAction(InputAction, TriggerEvent, this, &UGunnerInputEventDispatcherComponent::OnInputEvent, InputComponent, InputTag);
		}
	}
}
