// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerPlayerState.h"

#include "NexusActionComponent.h"
#include "Event/NexusEventManagerComponent.h"
#include "Gunner/Slot/GunnerSlotManagerComponent.h"
#include "Gunner/_Core/GunnerActionComponent.h"
#include "Net/UnrealNetwork.h"


AGunnerPlayerState::AGunnerPlayerState()
{
	NetUpdateFrequency = 100.0f;
	ActionComponent = CreateDefaultSubobject<UGunnerActionComponent>(TEXT("ActionComponent"));
	PredictionComponent = CreateDefaultSubobject<UNexusPredictionComponent>(TEXT("PredictionComponent"));
	CueComponent = CreateDefaultSubobject<UNexusCueComponent>(TEXT("CueComponent"));
	EventManagerComponent = CreateDefaultSubobject<UNexusEventManagerComponent>(TEXT("EventManagerComponent"));
	SlotManagerComponent = CreateDefaultSubobject<UGunnerSlotManagerComponent>(TEXT("SlotManagerComponent"));
}

void AGunnerPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (OnPawnSet.IsBound())
	{
		OnPawnSet.RemoveDynamic(this, &AGunnerPlayerState::OnPawnSetEvent);
	}
	OnPawnSet.AddDynamic(this, &AGunnerPlayerState::OnPawnSetEvent);
}

void AGunnerPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(AGunnerPlayerState, TeamID, COND_None, REPNOTIFY_Always);
}

void AGunnerPlayerState::SetGenericTeamId(const FGenericTeamId& InTeamID)
{
	FGenericTeamId OldTeamID = TeamID;
	TeamID = InTeamID;
	OnTeamSet.Broadcast(OldTeamID, TeamID);
}

void AGunnerPlayerState::OnPawnSetEvent(APlayerState* Player, APawn* NewPawn, APawn* OldPawn)
{
	ActionComponent->TeardownActionComponent();

	if (NewPawn)
	{
		ActionComponent->SetupActionComponent(NewPawn);
	}
}

void AGunnerPlayerState::OnRep_TeamID(FGenericTeamId OldTeamID)
{
	OnTeamSet.Broadcast(OldTeamID, TeamID);
}
