// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerSessionAsync_FindSession.h"

#include "GunnerSessionHelperSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"

UGunnerSessionAsync_FindSession* UGunnerSessionAsync_FindSession::FindSession(UObject* InWorldContextObject, FString InLobbyName)
{
	UGunnerSessionAsync_FindSession* SelfObject = NewObject<UGunnerSessionAsync_FindSession>(InWorldContextObject);
	SelfObject->WorldContextObject = InWorldContextObject;
	SelfObject->LobbyName = InLobbyName;
	return SelfObject;
}

void UGunnerSessionAsync_FindSession::Activate()
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
	SessionHelperSubsystem->OnFindSessionsCompleteDelegateMulticast.AddDynamic(this, &ThisClass::OnFindSessionComplete);
	SessionHelperSubsystem->FindSessions(LobbyName);
}

void UGunnerSessionAsync_FindSession::OnFindSessionComplete(bool bWasSuccessful, const TArray<FGunnerSessionLobbyInfo>& LobbyInfos)
{
	UGunnerSessionHelperSubsystem* SessionHelperSubsystem = WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UGunnerSessionHelperSubsystem>();
	check(SessionHelperSubsystem);
	SessionHelperSubsystem->OnFindSessionsCompleteDelegateMulticast.RemoveDynamic(this, &ThisClass::OnFindSessionComplete);
	OnCompleted.Broadcast(bWasSuccessful, LobbyInfos);
	Cancel();
}
