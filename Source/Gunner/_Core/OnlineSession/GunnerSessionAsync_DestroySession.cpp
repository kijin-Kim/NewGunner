// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerSessionAsync_DestroySession.h"

#include "GunnerSessionHelperSubsystem.h"

UGunnerSessionAsync_DestroySession* UGunnerSessionAsync_DestroySession::DestroySession(UObject* InWorldContextObject)
{
	UGunnerSessionAsync_DestroySession* SelfObject = NewObject<UGunnerSessionAsync_DestroySession>(InWorldContextObject);
	SelfObject->WorldContextObject = InWorldContextObject;
	return SelfObject;
}

void UGunnerSessionAsync_DestroySession::Activate()
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
	SessionHelperSubsystem->OnDestroySessionCompleteDelegateMulticast.AddDynamic(this, &ThisClass::OnDestroySessionComplete);
	SessionHelperSubsystem->DestroySession();
}

void UGunnerSessionAsync_DestroySession::OnDestroySessionComplete(FName InSessionName, bool bWasSuccessful)
{
	UGunnerSessionHelperSubsystem* SessionHelperSubsystem = WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UGunnerSessionHelperSubsystem>();
	check(SessionHelperSubsystem);
	SessionHelperSubsystem->OnDestroySessionCompleteDelegateMulticast.RemoveDynamic(this, &ThisClass::OnDestroySessionComplete);
	
	OnCompleted.Broadcast(InSessionName, bWasSuccessful);
	Cancel();
}
