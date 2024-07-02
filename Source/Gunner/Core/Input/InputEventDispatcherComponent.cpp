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
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	PC->OnPossessedPawnChanged.AddUniqueDynamic(this, &ThisClass::SetupInputEvent);
}

void UInputEventDispatcherComponent::OnInputEvent(UEnhancedInputComponent* InputComponent, FGameplayTag InputTag)
{
	if (!InputComponent)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	if (APawn* PossessedPawn = PC->GetPawn())
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

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	check(Subsystem);
	for (const auto& [IMC, Priority] : InputTagMappingData->InputContextAndPriorities)
	{
		Subsystem->AddMappingContext(IMC, Priority);
	}

	if (UEnhancedInputComponent* InputComponent = Cast<UEnhancedInputComponent>(PC->InputComponent))
	{
		for (const auto& [InputTag, InputAction] : InputTagMappingData->InputTagMappings)
		{
			InputComponent->BindAction(InputAction, ETriggerEvent::Triggered, this, &UInputEventDispatcherComponent::OnInputEvent, InputComponent, InputTag);
		}
	}


	FTimerHandle handle;
	GetWorld()->GetTimerManager().SetTimer(handle, [this]()
	{
		APlayerController* PC = Cast<APlayerController>(GetOwner());
		if (!PC || !PC->IsLocalController())
		{
			return;
		}

		if (APawn* PossessedPawn = PC->GetPawn())
		{
			UEventManagerComponent* EventManager = PossessedPawn->GetComponentByClass<UEventManagerComponent>();
			EventManager->BindEventCallback<FInputMessage>(FGameplayTag::RequestGameplayTag("Input.Test"), [](FGameplayTag InputTag, const FInputMessage& Message)
			{
				UE_LOG(LogGunner, Warning, TEXT("Input Test"));
			});

			EventManager->BindEventCallback<FInputMessage>(FGameplayTag::RequestGameplayTag("Input.Test"), this, &UInputEventDispatcherComponent::OnInputTest);
		}
	}, 2.0f, false);
}

void UInputEventDispatcherComponent::OnInputTest(FGameplayTag GameplayTag, const FInputMessage& InputMessage)
{
	UE_LOG(LogGunner, Warning, TEXT("Input Test2"));

}
