// Fill out your copyright notice in the Description page of Project Settings.


#include "NexusPlayerState.h"

#include "Action/NexusActionComponent.h"
#include "Action/SubComponent/NexusCueComponent.h"
#include "Action/SubComponent/NexusEventManagerComponent.h"
#include "Action/SubComponent/NexusGameplayTagComponent.h"
#include "Action/SubComponent/NexusPredictionComponent.h"
#include "Action/SubComponent/NexusPropertyComponent.h"
#include "Action/SubComponent/NexusSideEffectComponent.h"

ANexusPlayerState::ANexusPlayerState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bReplicates = true;
	SetNetUpdateFrequency(100.0f);
	ActionComponent = CreateDefaultSubobject<UNexusActionComponent>(ANexusPlayerState::ActionCompomentName);
	CueComponent = CreateDefaultSubobject<UNexusCueComponent>(TEXT("CueComponent"));
	CueComponent->SetIsReplicated(true);
	PropertyComponent = CreateDefaultSubobject<UNexusPropertyComponent>(TEXT("PropertyComponent"));
	PropertyComponent->SetIsReplicated(true);
	GameplayTagComponent = CreateDefaultSubobject<UNexusGameplayTagComponent>(TEXT("GameplayTagComponent"));
	GameplayTagComponent->SetIsReplicated(true);
	SideEffectComponent = CreateDefaultSubobject<UNexusSideEffectComponent>(TEXT("SideEffectComponent"));
	SideEffectComponent->SetIsReplicated(true);
	PredictionComponent = CreateDefaultSubobject<UNexusPredictionComponent>(TEXT("PredictionComponent"));
	PredictionComponent->SetIsReplicated(true);
	EventManagerComponent = CreateDefaultSubobject<UNexusEventManagerComponent>(TEXT("EventManagerComponent"));
}
