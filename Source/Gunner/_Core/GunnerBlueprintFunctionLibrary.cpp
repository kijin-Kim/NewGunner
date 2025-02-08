// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerBlueprintFunctionLibrary.h"

#include "GunnerLobbyGameState.h"

bool UGunnerBlueprintFunctionLibrary::IsTeamBoxSlotValid(const FTeamBoxSlot& Slot)
{
	return Slot.IsValid();
}
