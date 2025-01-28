// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerSessionAsync_JoinSession.h"

#include "GunnerSessionHelperSubsystem.h"

UGunnerSessionAsync_JoinSession* UGunnerSessionAsync_JoinSession::JoinSession(UObject* InWorldContextObject, APlayerController* InPlayerController, FString InSessionIdStr)
{
	UGunnerSessionAsync_JoinSession* SelfObject = NewObject<UGunnerSessionAsync_JoinSession>(InWorldContextObject);
	SelfObject->WorldContextObject = InWorldContextObject;
	SelfObject->PlayerController = InPlayerController;
	SelfObject->SeesionIdStr = InSessionIdStr;
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
	SessionHelperSubsystem->JoinSession(SeesionIdStr);
}

void UGunnerSessionAsync_JoinSession::OnJoinSessionComplete(FName SessionName, FString JoinSessionCompleteResult)
{
	UGunnerSessionHelperSubsystem* SessionHelperSubsystem = WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UGunnerSessionHelperSubsystem>();
	check(SessionHelperSubsystem);
	SessionHelperSubsystem->OnJoinSessionCompleteDelegateMulticast.RemoveDynamic(this, &ThisClass::OnJoinSessionComplete);
	
	OnCompleted.Broadcast(SessionName, JoinSessionCompleteResult);
	
	Cancel();
	const int32 LocalPlayerIndex = PlayerController->GetLocalPlayer()->GetLocalPlayerIndex();
	SessionHelperSubsystem->GetGameInstance()->ClientTravelToSession(LocalPlayerIndex, SessionName);
}
