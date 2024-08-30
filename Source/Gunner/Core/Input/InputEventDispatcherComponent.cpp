// Fill out your copyright notice in the Description page of Project Settings.


#include "InputEventDispatcherComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMessage.h"
#include "InputTagMappingData.h"
#include "Gunner/Gunner.h"
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

void UInputEventDispatcherComponent::OnInputEvent(UEnhancedInputComponent* InputComponent, FGameplayTag InputTag)
{
	if (!InputComponent)
	{
		return;
	}

	if (APawn* PossessedPawn = PlayerController->GetPawn())
	{
		UEventManagerComponent::SendEventToActor<FInputMessage>(InputTag, {}, PossessedPawn);
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

	if (UEnhancedInputComponent* InputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
	{
		for (const auto& [InputTag, InputAction] : InputTagMappingData->InputTagMappings)
		{
			InputComponent->BindAction(InputAction, ETriggerEvent::Triggered, this, &UInputEventDispatcherComponent::OnInputEvent, InputComponent, InputTag);
		}
	}
}
