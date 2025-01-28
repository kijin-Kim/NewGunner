// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerSessionAsync_JoinSession.h"

#include "GunnerSessionHelperSubsystem.h"

UGunnerSessionAsync_JoinSession* UGunnerSessionAsync_JoinSession::JoinSession(UObject* InWorldContextObject, APlayerController* InPlayerController, int32 InSessionResultIndex)
{
	UGunnerSessionAsync_JoinSession* SelfObject = NewObject<UGunnerSessionAsync_JoinSession>(InWorldContextObject);
	SelfObject->WorldContextObject = InWorldContextObject;
	SelfObject->PlayerController = InPlayerController;
	SelfObject->SessionResultIndex = InSessionResultIndex;
	return SelfObject;
}

void UGunnerSessionAsync_JoinSession::Activate()
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
	SessionHelperSubsystem->OnJoinSessionCompleteDelegateMulticast.AddDynamic(this, &ThisClass::OnJoinSessionComplete);
	SessionHelperSubsystem->JoinSession(SessionResultIndex);
}

void UGunnerSessionAsync_JoinSession::OnJoinSessionComplete(FName SessionName, FString JoinSessionCompleteResult)
{
	UGunnerSessionHelperSubsystem* SessionHelperSubsystem = WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UGunnerSessionHelperSubsystem>();
	check(SessionHelperSubsystem);
	OnCompleted.Broadcast(SessionName, JoinSessionCompleteResult);
	Cancel();
	const int32 LocalPlayerIndex = PlayerController->GetLocalPlayer()->GetLocalPlayerIndex();
	if (!SessionHelperSubsystem->GetGameInstance()->ClientTravelToSession(LocalPlayerIndex, SessionName))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to travel to session %s"), *SessionName.ToString());
	}
}
