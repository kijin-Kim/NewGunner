// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerSessionAsync_CancelFindSession.h"

UGunnerSessionAsync_CancelFindSession* UGunnerSessionAsync_CancelFindSession::CancelFindSession(UObject* InWorldContextObject)
{
	UGunnerSessionAsync_CancelFindSession* SelfObject = NewObject<UGunnerSessionAsync_CancelFindSession>(InWorldContextObject);
	SelfObject->WorldContextObject = InWorldContextObject;

	return SelfObject;
}

void UGunnerSessionAsync_CancelFindSession::Activate()
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
	SessionHelperSubsystem->OnCancelFindSessionsCompleteDelegateMulticast.AddDynamic(this, &ThisClass::OnCancelFindSessionComplete);
	SessionHelperSubsystem->CancelFindSessions();
}

void UGunnerSessionAsync_CancelFindSession::OnCancelFindSessionComplete(bool bWasSuccessful)
{
	UGunnerSessionHelperSubsystem* SessionHelperSubsystem = WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UGunnerSessionHelperSubsystem>();
	check(SessionHelperSubsystem);
	SessionHelperSubsystem->OnCancelFindSessionsCompleteDelegateMulticast.RemoveDynamic(this, &ThisClass::OnCancelFindSessionComplete);
	OnCompleted.Broadcast(bWasSuccessful);

	Cancel();
}
