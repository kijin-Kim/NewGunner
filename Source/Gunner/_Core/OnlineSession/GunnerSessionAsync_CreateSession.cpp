// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerSessionAsync_CreateSession.h"

#include "GunnerSessionHelperSubsystem.h"

UGunnerSessionAsync_CreateSession* UGunnerSessionAsync_CreateSession::CreateSession(UObject* InWorldContextObject, FString InLobbyName, FString InMapName, int32 InMaxNumPlayers)
{
	UGunnerSessionAsync_CreateSession* SelfObject = NewObject<UGunnerSessionAsync_CreateSession>(InWorldContextObject);
	SelfObject->WorldContextObject = InWorldContextObject;
	SelfObject->LobbyName = InLobbyName;
	SelfObject->MapName = InMapName;
	SelfObject->MaxNumPlayers = InMaxNumPlayers;
	return SelfObject;
}

void UGunnerSessionAsync_CreateSession::Activate()
{
	Super::Activate();
	if (!ensure(WorldContextObject.IsValid()))
	{
		return;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject.Get(), EGetWorldErrorMode::LogAndReturnNull);
	check(World);
	UGameInstance* GameInstance = World->GetGameInstance();
	check(GameInstance);

	UGunnerSessionHelperSubsystem* SessionHelperSubsystem = GameInstance->GetSubsystem<UGunnerSessionHelperSubsystem>();
	check(SessionHelperSubsystem);
	SessionHelperSubsystem->OnCreateSessionCompleteDelegateMulticast.AddDynamic(this, &ThisClass::OnCreateSessionComplete);
	SessionHelperSubsystem->CreateSession(LobbyName, MapName, MaxNumPlayers);
}

void UGunnerSessionAsync_CreateSession::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UGunnerSessionHelperSubsystem* SessionHelperSubsystem = WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UGunnerSessionHelperSubsystem>();
	check(SessionHelperSubsystem);
	SessionHelperSubsystem->OnCreateSessionCompleteDelegateMulticast.RemoveDynamic(this, &ThisClass::OnCreateSessionComplete);
	OnCompleted.Broadcast(SessionName, bWasSuccessful);
	Cancel();
}
