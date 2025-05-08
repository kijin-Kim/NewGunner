// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerPlayerState.h"

#include "Action/NexusActionComponent.h"
#include "Gunner/Gunner.h"
#include "Gunner/Action/GunnerActionSet.h"
#include "Gunner/Item/GunnerInventoryManagerComponent.h"
#include "Gunner/_Core/GunnerActionComponent.h"
#include "Gunner/_Core/GunnerBlueprintFunctionLibrary.h"
#include "Net/UnrealNetwork.h"


AGunnerPlayerState::AGunnerPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UGunnerActionComponent>(ANexusPlayerState::ActionCompomentName))
{
	InventoryManagerComponent = CreateDefaultSubobject<UGunnerInventoryManagerComponent>(TEXT("InventoryManagerComponent"));
	InventoryManagerComponent->SetIsReplicated(true);
}

void AGunnerPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		 AuthRemoveActionSets();
	}
	Super::EndPlay(EndPlayReason);
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
	if (NewPawn)
	{
		ActionComponent->SetupActionComponent(NewPawn);
		if (HasAuthority())
		{
			AuthAddActionSets();
		}
	}
}

void AGunnerPlayerState::OnRep_TeamID(FGenericTeamId OldTeamID)
{
	OnTeamSet.Broadcast(OldTeamID, TeamID);
}

void AGunnerPlayerState::AuthAddActionSets()
{
	if (!HasAuthority())
	{
		GR_LOG_SUB(this, LogGunner, Error, TEXT("권한 없는 함수 호출"));
		return;
	}


	check(ActionComponent);
	for (const UGunnerActionSet* ActionSet : ActionSets)
	{
		UGunnerBlueprintFunctionLibrary::AuthAddDesiredActions(ActionComponent->GetAgentActor(), ActionComponent->GetAgentActor(), ActionSet->ActionClasses, AddedActionHandles);
		UGunnerBlueprintFunctionLibrary::AuthAddDesiredItems(ActionComponent->GetAgentActor(), ActionSet->ItemDefinitions, AddedItems);
	}
}

void AGunnerPlayerState::AuthRemoveActionSets()
{
	if (!HasAuthority())
	{
		GR_LOG_SUB(this, LogGunner, Error, TEXT("권한 없는 함수 호출"));
		return;
	}

	UGunnerBlueprintFunctionLibrary::AuthRemoveDesiredActions(ActionComponent->GetAgentActor(), AddedActionHandles);
	AddedActionHandles.Empty();

	UGunnerBlueprintFunctionLibrary::AuthRemoveDesiredItems(ActionComponent->GetAgentActor(), AddedItems, true);
	AddedItems.Empty();
}
