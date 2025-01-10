// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerMainMenuWidget.h"

#include "GunnerButtonWidget.h"
#include "OnlineSessionSettings.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Gunner/Gunner.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Online/OnlineSessionNames.h"

void UGunnerMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	check(OnlineSubsystem);
	SessionInterfacePtr = OnlineSubsystem->GetSessionInterface();
	check(SessionInterfacePtr);
}

void UGunnerMainMenuWidget::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	SessionInterfacePtr->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
	if (bWasSuccessful)
	{
		// log session name room name map name
		FString RoomName;
		FString MapName;
		SessionInterfacePtr->GetNamedSession(NAME_GameSession)->SessionSettings.Get(FName("ROOM_NAME"), RoomName);
		SessionInterfacePtr->GetNamedSession(NAME_GameSession)->SessionSettings.Get(FName("MAP_NAME"), MapName);
		UE_LOG(LogGunner, Verbose, TEXT("세션 [%s] 생성 성공. 방 이름 [%], 맵 이름 [%s]"), *SessionName.ToString(), *RoomName, *MapName);

		GetWorld()->ServerTravel("/Game/Maps/FirstPersonMap?listen");
		return;
	}

	UE_LOG(LogGunner, Error, TEXT("세션 생성 실패"));
}


void UGunnerMainMenuWidget::OnFindSessionsComplete(bool bWasSuccessful)
{
	SessionInterfacePtr->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);
	TArray<FRoomInfo> RoomInfos;
	if (bWasSuccessful && SessionSearch.IsValid())
	{
		for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
		{
			FString RoomName;
			FString MapName;
			SearchResult.Session.SessionSettings.Get(FName("ROOM_NAME"), RoomName);
			SearchResult.Session.SessionSettings.Get(FName("MAP_NAME"), MapName);
			FRoomInfo NewRoomInfo{RoomName, MapName, SearchResult.PingInMs};
			RoomInfos.Add(NewRoomInfo);
			UE_LOG(LogGunner, Verbose, TEXT("%s"), *NewRoomInfo.ToString());
		}
	}
	else
	{
		UE_LOG(LogGunner, Error, TEXT("세션 검색 실패"));
	}

	if (!RoomInfos.IsEmpty())
	{
		RoomInfos.Sort([](const FRoomInfo& A, const FRoomInfo& B) { return A.PingInMs < B.PingInMs; });
	}

	OnSessionFindCompleted.Broadcast(RoomInfos);
}

void UGunnerMainMenuWidget::OnJoinSessionComplete(FName Name, EOnJoinSessionCompleteResult::Type Arg)
{
	SessionInterfacePtr->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
	if (Arg == EOnJoinSessionCompleteResult::Success)
	{
		FString Address;
		SessionInterfacePtr->GetResolvedConnectString(NAME_GameSession, Address);
		GetOwningPlayer()->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
	}
	else
	{
		UE_LOG(LogGunner, Error, TEXT("Session join failed"));
	}
}

void UGunnerMainMenuWidget::FindSessions()
{
	if (GIsPlayInEditorWorld)
	{
		UE_LOG(LogGunner, Error, TEXT("에디터에서 실행 중에는 사용할 수 없습니다."));
		OnFindSessionsComplete(false);
		return;
	}

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	SessionSearch->MaxSearchResults = 10000;
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	OnFindSessionsCompleteDelegateHandle = SessionInterfacePtr->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateUObject(this, &UGunnerMainMenuWidget::OnFindSessionsComplete));

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterfacePtr->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef()))
	{
		SessionInterfacePtr->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);
		OnFindSessionsComplete(false);
	}
}

void UGunnerMainMenuWidget::OnHostButtonClicked(FString RoomName, FString MapName)
{
	if (GIsPlayInEditorWorld)
	{
		UE_LOG(LogGunner, Error, TEXT("Cannot host in editor world"));
		return;
	}

	check(!MapName.IsEmpty());

	if (SessionInterfacePtr->GetNamedSession(NAME_GameSession))
	{
		SessionInterfacePtr->DestroySession(NAME_GameSession);
	}
	OnCreateSessionCompleteDelegateHandle = SessionInterfacePtr->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &UGunnerMainMenuWidget::OnCreateSessionComplete));

	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	SessionSettings.bUseLobbiesIfAvailable = true;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.NumPublicConnections = 2;
	SessionSettings.BuildUniqueId = 1;

	SessionSettings.Set(FName("SESSION_NAME"), RoomName, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(FName("MAP_NAME"), MapName, EOnlineDataAdvertisementType::ViaOnlineService);

	if (!SessionInterfacePtr->CreateSession(0, NAME_GameSession, SessionSettings))
	{
		SessionInterfacePtr->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
		OnCreateSessionComplete(NAME_None, false);
	}
}

void UGunnerMainMenuWidget::OnShutdownButtonClicked()
{
	if (SessionInterfacePtr->GetNamedSession(NAME_GameSession))
	{
		SessionInterfacePtr->DestroySession(NAME_GameSession);
	}

	UKismetSystemLibrary::QuitGame(GetWorld(), GetOwningPlayer(), EQuitPreference::Quit, false);
}


void UGunnerMainMenuWidget::JoinSession()
{
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	OnJoinSessionCompleteDelegateHandle = SessionInterfacePtr->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UGunnerMainMenuWidget::OnJoinSessionComplete));
	if (!SessionInterfacePtr->JoinSession(LocalPlayer->GetControllerId(), NAME_GameSession, SessionSearch->SearchResults[0]))
	{
		SessionInterfacePtr->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
		OnJoinSessionComplete(NAME_None, EOnJoinSessionCompleteResult::UnknownError);
	}
}
