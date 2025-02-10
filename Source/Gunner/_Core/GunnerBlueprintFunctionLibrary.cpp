// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerBlueprintFunctionLibrary.h"

#include "GunnerLobbyGameState.h"
#include "GameFramework/PlayerState.h"

bool UGunnerBlueprintFunctionLibrary::IsTeamBoxSlotValid(const FTeamBoxSlot& Slot)
{
	return Slot.IsValid();
}

void UGunnerBlueprintFunctionLibrary::ServerTravelBySoftObjectPtr(const UObject* WorldContextObject, const TSoftObjectPtr<UWorld> Level, bool bAbsolute, FString Options)
{
	const FName LevelName = FName(*FPackageName::ObjectPathToPackageName(Level.ToString()));
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	FString URL = LevelName.ToString();
	URL += FString::Printf(TEXT("?%s"), *Options);
	World->ServerTravel(URL, bAbsolute);
}

ETeamAttitude::Type UGunnerBlueprintFunctionLibrary::GetTeamAttitude(APlayerState* PlayerState, APlayerState* OtherPlayerState)
{
	if (PlayerState == nullptr || OtherPlayerState == nullptr)
	{
		return ETeamAttitude::Neutral;
	}

	IGenericTeamAgentInterface* TeamAgentInterface = Cast<IGenericTeamAgentInterface>(PlayerState);
	if (TeamAgentInterface == nullptr)
	{
		return ETeamAttitude::Neutral;
	}
	
	return TeamAgentInterface->GetTeamAttitudeTowards(*OtherPlayerState);
}
