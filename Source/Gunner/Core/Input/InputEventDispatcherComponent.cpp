// Fill out your copyright notice in the Description page of Project Settings.


#include "InputEventDispatcherComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMessage.h"
#include "InputTagMappingData.h"
#include "Gunner/Core/Event/EventManagerComponent.h"


UInputEventDispatcherComponent::UInputEventDispatcherComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void UInputEventDispatcherComponent::InitializeComponent()
{
	Super::InitializeComponent();
	PlayerController = Cast<APlayerController>(GetOwner());
	if (PlayerController && PlayerController->IsLocalController())
	{
		PlayerController->OnPossessedPawnChanged.AddUniqueDynamic(this, &ThisClass::SetupInputEvent);
	}
}

void UInputEventDispatcherComponent::OnInputEvent(const FInputActionValue& InputActionValue, UEnhancedInputComponent* InputComponent, FGameplayTag InputTag)
{
	if (!InputComponent)
	{
		return;
	}

	if (APawn* PossessedPawn = PlayerController->GetPawn())
	{
		FInputMessage InputMessage;
		InputMessage.Value = InputActionValue;
		UEventManagerComponent::SendEventToActor<FInputMessage>(InputTag, InputMessage, PossessedPawn);
	}
}

void UInputEventDispatcherComponent::SetupInputEvent(APawn* OldPawn, APawn* NewPawn)
{
	if (!InputTagMappingData || !NewPawn)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	check(Subsystem);
	for (const auto& [IMC, Priority] : InputTagMappingData->InputContextAndPriorities)
	{
		Subsystem->AddMappingContext(IMC, Priority);
	}

	UEnhancedInputComponent* InputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent);
	if(!InputComponent)
	{
		return;
	}

	for (const auto& [InputAction, TriggerEventMappings] : InputTagMappingData->InputTagMappings)
	{
		for(const auto& [TriggerEvent, InputTag] : TriggerEventMappings)
		{
			InputComponent->BindAction(InputAction, TriggerEvent, this, &UInputEventDispatcherComponent::OnInputEvent, InputComponent, InputTag);	
		}
	}
}
