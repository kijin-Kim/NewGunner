// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerMainMenuWidget.h"

#include "GunnerButtonWidget.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "GameFramework/PlayerState.h"
#include "Gunner/Gunner.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Online/OnlineSessionNames.h"


UGunnerMainMenuWidget::UGunnerMainMenuWidget()
	: OnSessionParticipantJoinedDelegate(FOnSessionParticipantJoinedDelegate::CreateUObject(this, &UGunnerMainMenuWidget::OnSessionParticipantJoined)),
	  OnSessionParticipantLeftDelegate(FOnSessionParticipantLeftDelegate::CreateUObject(this, &UGunnerMainMenuWidget::OnSessionParticipantLeft)),
	  OnCreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &UGunnerMainMenuWidget::OnCreateSessionComplete)),
	  OnFindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &UGunnerMainMenuWidget::OnFindSessionsComplete)),
	  OnJoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UGunnerMainMenuWidget::OnJoinSessionComplete)),
	  OnStartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &UGunnerMainMenuWidget::OnStartSessionComplete))
{
}

void UGunnerMainMenuWidget::OnSessionParticipantJoined(FName Name, const FUniqueNetId& UniqueNetId)
{
	if (Name == NAME_GameSession)
	{
		IOnlineSessionPtr SessionInterfacePtr = Online::GetSubsystem(GetWorld())->GetSessionInterface();
		OnJoinedSessionParticipantsChanged.Broadcast(GetParticipants(SessionInterfacePtr->GetNamedSession(NAME_GameSession)));
		UE_LOG(LogGunner, Log, TEXT("플레이어 [%s]가 세션에 참가하였습니다."), *UniqueNetId.ToString());
	}
}

void UGunnerMainMenuWidget::OnSessionParticipantLeft(FName Name, const FUniqueNetId& UniqueNetId, EOnSessionParticipantLeftReason OnSessionParticipantLeftReason)
{
	if (Name == NAME_GameSession)
	{
		IOnlineSessionPtr SessionInterfacePtr = Online::GetSubsystem(GetWorld())->GetSessionInterface();
		OnJoinedSessionParticipantsChanged.Broadcast(GetParticipants(SessionInterfacePtr->GetNamedSession(NAME_GameSession)));
		UE_LOG(LogGunner, Log, TEXT("플레이어가 세션을 떠났습니다: %s 사유: %s"), *UniqueNetId.ToString(), *ToLogString(OnSessionParticipantLeftReason));
	}
}

void UGunnerMainMenuWidget::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSessionPtr SessionInterfacePtr = Online::GetSubsystem(GetWorld())->GetSessionInterface();
	OnSessionParticipantJoinedDelegateHandle = SessionInterfacePtr->AddOnSessionParticipantJoinedDelegate_Handle(OnSessionParticipantJoinedDelegate);
	OnSessionParticipantLeftDelegateHandle = SessionInterfacePtr->AddOnSessionParticipantLeftDelegate_Handle(OnSessionParticipantLeftDelegate);


	SessionInterfacePtr->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);

	if (bWasSuccessful)
	{
		FNamedOnlineSession* Session = SessionInterfacePtr->GetNamedSession(SessionName);
		check(Session);

		FString RoomName;
		FString MapName;
		Session->SessionSettings.Get(TEXT("ROOM_NAME"), RoomName);
		Session->SessionSettings.Get(TEXT("MAP_NAME"), MapName);


		UE_LOG(LogGunner, Log, TEXT("세션 [%s] 생성 성공. 방 이름 [%s], 맵 이름 [%s]"), *SessionName.ToString(), *RoomName, *MapName);
		FRoomInfo RoomInfo{};
		RoomInfo.RoomName = RoomName;
		RoomInfo.MapName = MapName;
		RoomInfo.PlayerCount = 0;
		RoomInfo.MaxPlayerCount = Session->SessionSettings.NumPublicConnections;
		RoomInfo.PingInMs = 0;
		RoomInfo.SessionId = Session->GetSessionIdStr();
		RoomInfo.Participants = GetParticipants(Session);

		OnJoinSessionLobbySucceeded.Broadcast(RoomInfo);
		GetWorld()->ServerTravel("/Game/Maps/Lobby?listen");
		return;
	}

	UE_LOG(LogGunner, Error, TEXT("세션 생성 실패"));
}


void UGunnerMainMenuWidget::OnFindSessionsComplete(bool bWasSuccessful)
{
	IOnlineSessionPtr SessionInterface = Online::GetSubsystem(GetWorld())->GetSessionInterface();
	IOnlineSessionPtr SessionInterfacePtr = Online::GetSubsystem(GetWorld())->GetSessionInterface();

	SessionInterfacePtr->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);
	TArray<FRoomInfo> RoomInfos;
	if (bWasSuccessful && SessionSearch.IsValid())
	{
		for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
		{
			if (!SearchResult.IsSessionInfoValid())
			{
				continue;
			}

			FString RoomName;
			FString MapName;
			SearchResult.Session.SessionSettings.Get(TEXT("ROOM_NAME"), RoomName);
			SearchResult.Session.SessionSettings.Get(TEXT("MAP_NAME"), MapName);
			RoomName = DecodeString(RoomName);
			MapName = DecodeString(MapName);

			
			FRoomInfo NewRoomInfo;
			NewRoomInfo.RoomName = RoomName;
			NewRoomInfo.MapName = MapName;
			NewRoomInfo.PlayerCount = SearchResult.Session.NumOpenPublicConnections;
			NewRoomInfo.MaxPlayerCount = SearchResult.Session.SessionSettings.NumPublicConnections;
			NewRoomInfo.PingInMs = SearchResult.PingInMs;
			NewRoomInfo.SessionId = SearchResult.GetSessionIdStr();
			
			RoomInfos.Add(NewRoomInfo);
			UE_LOG(LogGunner, Log, TEXT("%s"), *NewRoomInfo.ToString());
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
	IOnlineSessionPtr SessionInterfacePtr = Online::GetSubsystem(GetWorld())->GetSessionInterface();


	SessionInterfacePtr->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
	if (Arg == EOnJoinSessionCompleteResult::Success)
	{
		FRoomInfo RoomInfo;
		FNamedOnlineSession* Session = SessionInterfacePtr->GetNamedSession(NAME_GameSession);
		check(Session);
		FString RoomName;
		FString MapName;

		Session->SessionSettings.Get(TEXT("ROOM_NAME"), RoomName);
		Session->SessionSettings.Get(TEXT("MAP_NAME"), MapName);
		RoomInfo.RoomName = DecodeString(RoomName);
		RoomInfo.MapName = DecodeString(MapName);


		RoomInfo.PlayerCount = Session->RegisteredPlayers.Num();
		RoomInfo.MaxPlayerCount = Session->SessionSettings.NumPublicConnections;

		//RoomInfo.PingInMs = TODO 
		RoomInfo.SessionId = Session->GetSessionIdStr();
		RoomInfo.Participants = GetParticipants(Session);

		OnJoinSessionLobbySucceeded.Broadcast(RoomInfo);
		UE_LOG(LogGunner, Log, TEXT("세션 [%s] 참가 성공. 방 이름 [%s], 맵 이름 [%s]"), *Name.ToString(), *RoomName, *MapName);
		FString Address;
		SessionInterfacePtr->GetResolvedConnectString(NAME_GameSession, Address);
		if (APlayerController* PC = GetOwningPlayer())
		{
			PC->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
		}
	}
	else
	{
		UE_LOG(LogGunner, Error, TEXT("세션 참가 실패"));
	}
}

void UGunnerMainMenuWidget::OnStartSessionComplete(FName Name, bool bArg)
{
	IOnlineSessionPtr SessionInterfacePtr = Online::GetSubsystem(GetWorld())->GetSessionInterface();
	SessionInterfacePtr->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);

	if (bArg)
	{
		UE_LOG(LogGunner, Log, TEXT("세션 [%s] 시작 성공"), *Name.ToString());
	}
	else
	{
		UE_LOG(LogGunner, Error, TEXT("세션 시작 실패"));
	}
}

void UGunnerMainMenuWidget::FindSession(FString RoomName)
{
	if (GIsPlayInEditorWorld && Online::GetSubsystem(GetWorld())->GetSubsystemName() != "NULL")
	{
		UE_LOG(LogGunner, Error, TEXT("에디터에서 실행 중에는 사용할 수 없습니다."));
		OnFindSessionsComplete(false);
		return;
	}

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = Online::GetSubsystem(GetWorld())->GetSubsystemName() == "NULL" ? true : false;
	SessionSearch->MaxSearchResults = 10000;
	// SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	// if (!RoomName.IsEmpty())
	// {
	// 	SessionSearch->QuerySettings.Set(FName(TEXT("ROOM_NAME")), RoomName, EOnlineComparisonOp::Equals);
	// }
	//
	// SessionSearch->QuerySettings.Set(FName(TEXT("HI_FELLOW_DEVS")), FString("TESTING"), EOnlineComparisonOp::Equals);

	IOnlineSessionPtr SessionInterfacePtr = Online::GetSubsystem(GetWorld())->GetSessionInterface();

	OnFindSessionsCompleteDelegateHandle = SessionInterfacePtr->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterfacePtr->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef()))
	{
		SessionInterfacePtr->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);
		OnFindSessionsComplete(false);
	}
}

void UGunnerMainMenuWidget::LeaveSession()
{
	IOnlineSessionPtr SessionInterfacePtr = Online::GetSubsystem(GetWorld())->GetSessionInterface();

	if (FNamedOnlineSession* NamedOnlineSession = SessionInterfacePtr->GetNamedSession(NAME_GameSession))
	{
		if (NamedOnlineSession->bHosting)
		{
			SessionInterfacePtr->ClearOnSessionParticipantJoinedDelegate_Handle(OnSessionParticipantJoinedDelegateHandle);
			SessionInterfacePtr->ClearOnSessionParticipantLeftDelegate_Handle(OnSessionParticipantLeftDelegateHandle);
		}

		SessionInterfacePtr->DestroySession(NAME_GameSession);
	}
}


bool UGunnerMainMenuWidget::IsLocalPlayerHost() const
{
	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	if (!OnlineSubsystem) return false;

	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid()) return false;

	FNamedOnlineSession* CurrentSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (!CurrentSession) return false;

	IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();
	if (!IdentityInterface.IsValid()) return false;

	TSharedPtr<const FUniqueNetId> LocalUserId = IdentityInterface->GetUniquePlayerId(0); // 로컬 플레이어 ID
	if (!LocalUserId.IsValid()) return false;

	// 호스트의 UserID와 로컬 플레이어의 UserID 비교
	return CurrentSession->OwningUserId->ToString() == LocalUserId->ToString();
}

FString UGunnerMainMenuWidget::GetPlayerNickname(const FUniqueNetId& UserId) const
{
	check(UserId.IsValid());
	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	check(OnlineSubsystem);

	IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();
	check(IdentityInterface);

	return IdentityInterface->GetPlayerNickname(UserId);
}

TArray<FString> UGunnerMainMenuWidget::GetParticipants(FNamedOnlineSession* Session) const
{
	TArray<FString> Participants;
	Participants.Add(Session->OwningUserName);
	for (const FUniqueNetIdRef& PlayerId : Session->RegisteredPlayers)
	{
		Participants.Add(PlayerId->ToString());
	}
	return Participants;
}


FString UGunnerMainMenuWidget::DecodeString(const FString& TargetString) const
{
	TArray<uint8> Utf8Bytes;
	// 잘못된 UTF-16 문자열을 UTF-8로 복원
	for (TCHAR Char : TargetString)
	{
		Utf8Bytes.Add(static_cast<uint8>(Char & 0xFF)); // 하위 8비트 추출
	}
	// UTF-8 문자열이 끝났음을 알리기 위해 널 종료 추가
	Utf8Bytes.Add('\0');
	// UTF-8 데이터를 UTF-16로 디코딩하여 FString으로 변환
	FString RestoredString = UTF8_TO_TCHAR(reinterpret_cast<const char*>(Utf8Bytes.GetData()));
	return RestoredString;
}


void UGunnerMainMenuWidget::OnHostButtonClicked(FString RoomName, FString MapName)
{
	if (GIsPlayInEditorWorld && Online::GetSubsystem(GetWorld())->GetSubsystemName() != "NULL")
	{
		UE_LOG(LogGunner, Error, TEXT("에디터에서 실행 중에는 사용할 수 없습니다."));
		return;
	}


	check(!MapName.IsEmpty());

	LeaveSession();
	IOnlineSessionPtr SessionInterfacePtr = Online::GetSubsystem(GetWorld())->GetSessionInterface();

	OnCreateSessionCompleteDelegateHandle = SessionInterfacePtr->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);


	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = Online::GetSubsystem(GetWorld())->GetSubsystemName() == "NULL" ? true : false;
	SessionSettings.NumPrivateConnections = 0;
	SessionSettings.bUseLobbiesIfAvailable = true;
	SessionSettings.bIsDedicated = false;
	SessionSettings.bUsesStats = false;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bAllowInvites = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bAllowJoinViaPresence = true;
	SessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
	SessionSettings.NumPublicConnections = 3;
	SessionSettings.BuildUniqueId = 0;


	SessionSettings.Set(FName(TEXT("ROOM_NAME")), RoomName, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(FName(TEXT("MAP_NAME")), MapName, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(FName(TEXT("HI_FELLOW_DEVS")), FString("TESTING"), EOnlineDataAdvertisementType::ViaOnlineService);


	auto PC = GetWorld()->GetFirstPlayerController();
	APlayerState* PS = PC->PlayerState;

	if (!SessionInterfacePtr->CreateSession(*PS->GetUniqueId().GetUniqueNetId(), NAME_GameSession, SessionSettings))
	{
		SessionInterfacePtr->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
		OnCreateSessionComplete(NAME_None, false);
	}
}

void UGunnerMainMenuWidget::OnShutdownButtonClicked()
{
	LeaveSession();
	UKismetSystemLibrary::QuitGame(GetWorld(), GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UGunnerMainMenuWidget::StartGame()
{
	// Get Session and update session state
	IOnlineSessionPtr SessionInterfacePtr = Online::GetSubsystem(GetWorld())->GetSessionInterface();

	FNamedOnlineSession* Session = SessionInterfacePtr->GetNamedSession(NAME_GameSession);
	check(Session);
	Session->SessionState = EOnlineSessionState::InProgress;

	GetWorld()->ServerTravel("/Game/Maps/FirstPersonMap?listen");
}

bool UGunnerMainMenuWidget::CanStartGame() const
{
	IOnlineSessionPtr SessionInterfacePtr = Online::GetSubsystem(GetWorld())->GetSessionInterface();

	FNamedOnlineSession* Session = SessionInterfacePtr->GetNamedSession(NAME_GameSession);
	return Session && Session->RegisteredPlayers.Num() > 1;
}


void UGunnerMainMenuWidget::JoinSession(FString SessionId)
{
	LeaveSession();
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	IOnlineSessionPtr SessionInterfacePtr = Online::GetSubsystem(GetWorld())->GetSessionInterface();

	OnJoinSessionCompleteDelegateHandle = SessionInterfacePtr->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
	FOnlineSessionSearchResult* SearchResultPtr = SessionSearch->SearchResults.FindByPredicate([&SessionId](const FOnlineSessionSearchResult& SearchResult)
	{
		return SearchResult.GetSessionIdStr() == SessionId;
	});
	check(SearchResultPtr);

	if (!SessionInterfacePtr->JoinSession(LocalPlayer->GetControllerId(), NAME_GameSession, *SearchResultPtr))
	{
		SessionInterfacePtr->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
		OnJoinSessionComplete(NAME_None, EOnJoinSessionCompleteResult::UnknownError);
	}
}
