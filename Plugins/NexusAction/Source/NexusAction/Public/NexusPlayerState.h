// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "NexusPlayerState.generated.h"

class UNexusEventManagerComponent;
class UNexusCueComponent;
class UNexusGameplayTagComponent;
class UNexusPropertyComponent;
class UNexusSideEffectComponent;
class UNexusPredictionComponent;
class UNexusActionComponent;
/**
 * 
 */
UCLASS()
class NEXUSACTION_API ANexusPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ANexusPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	static inline FName ActionCompomentName = TEXT("ActionComponent");

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNexusActionComponent> ActionComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNexusPredictionComponent> PredictionComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNexusSideEffectComponent> SideEffectComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNexusPropertyComponent> PropertyComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNexusGameplayTagComponent> GameplayTagComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNexusCueComponent> CueComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNexusEventManagerComponent> EventManagerComponent;
	
};
