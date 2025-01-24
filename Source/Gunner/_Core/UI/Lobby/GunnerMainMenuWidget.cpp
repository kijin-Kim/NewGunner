// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerMainMenuWidget.h"

#include "GunnerButtonWidget.h"
#include "OnlineSessionSettings.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Gunner/Gunner.h"
#include "Interfaces/OnlineIdentityInterface.h"
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

	SessionInterfacePtr->OnSessionParticipantsChangeDelegates.AddUObject(this, &ThisClass::OnParticipantsChanged);
}

void UGunnerMainMenuWidget::OnParticipantsChanged(FName SessionName, const FUniqueNetId& UniqueNetId, bool bJoined)
{
	UE_LOG(LogGunner, Verbose, TEXT("OnParticipantsChanged: %s, %s, %d"), *SessionName.ToString(), *UniqueNetId.ToString(), bJoined);
	FNamedOnlineSession* NamedOnlineSession = SessionInterfacePtr->GetNamedSession(NAME_GameSession);
	check(NamedOnlineSession);
	OnJoinedSessionParticipantsChanged.Broadcast(GetParticipants(NamedOnlineSession));
}


void UGunnerMainMenuWidget::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	SessionInterfacePtr->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
	if (bWasSuccessful)
	{
		FNamedOnlineSession* Session = SessionInterfacePtr->GetNamedSession(SessionName);
		check(Session);

		FString RoomName;
		FString MapName;
		Session->SessionSettings.Get(TEXT("ROOM_NAME"), RoomName);
		Session->SessionSettings.Get(TEXT("MAP_NAME"), MapName);

		UE_LOG(LogGunner, Verbose, TEXT("세션 [%s] 생성 성공. 방 이름 [%s], 맵 이름 [%s]"), *SessionName.ToString(), *RoomName, *MapName);
		FRoomInfo RoomInfo{
			RoomName,
			MapName,
			0,
			Session->SessionSettings.NumPublicConnections,
			0,
			Session->GetSessionIdStr()
		};
		OnJoinSessionLobbySucceeded.Broadcast(RoomInfo);
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


			FRoomInfo NewRoomInfo{
				RoomName,
				MapName,
				SearchResult.Session.SessionSettings.NumPublicConnections - SearchResult.Session.NumOpenPublicConnections,
				SearchResult.Session.SessionSettings.NumPublicConnections,
				SearchResult.PingInMs,
				SearchResult.GetSessionIdStr()
			};
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
	}
	else
	{
		UE_LOG(LogGunner, Error, TEXT("Session join failed"));
	}
}

void UGunnerMainMenuWidget::FindSession(FString RoomName)
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
	if (!RoomName.IsEmpty())
	{
		SessionSearch->QuerySettings.Set(FName(TEXT("ROOM_NAME")), RoomName, EOnlineComparisonOp::Equals);
	}

	OnFindSessionsCompleteDelegateHandle = SessionInterfacePtr->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateUObject(this, &UGunnerMainMenuWidget::OnFindSessionsComplete));

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterfacePtr->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef()))
	{
		SessionInterfacePtr->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);
		OnFindSessionsComplete(false);
	}
}

bool UGunnerMainMenuWidget::IsLocalPlayerHost() const
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
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
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	check(OnlineSubsystem);

	IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();
	check(IdentityInterface);

	return IdentityInterface->GetPlayerNickname(UserId);
}

TArray<FString> UGunnerMainMenuWidget::GetParticipants(FNamedOnlineSession* Session) const
{
	TArray<FString> Participants;
	Participants.Reserve(Session->RegisteredPlayers.Num() + 1);
	Participants.Add(GetPlayerNickname(*Session->OwningUserId));
	for (const FUniqueNetIdRef& PlayerId : Session->RegisteredPlayers)
	{
		Participants.Add(GetPlayerNickname(*PlayerId));
	}
	return Participants;
}

FString UGunnerMainMenuWidget::EncodeString(const FString& TargetString) const
{
	return std::string(TCHAR_TO_UTF8(*TargetString)).c_str();
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

	SessionSettings.Set(FName(TEXT("ROOM_NAME")), RoomName, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(FName(TEXT("MAP_NAME")), MapName, EOnlineDataAdvertisementType::ViaOnlineService);


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

void UGunnerMainMenuWidget::StartGame()
{
	// Get Session and update session state
	FNamedOnlineSession* Session = SessionInterfacePtr->GetNamedSession(NAME_GameSession);
	check(Session);
	Session->SessionState = EOnlineSessionState::InProgress;
	GetWorld()->ServerTravel("/Game/Maps/FirstPersonMap?listen");
}

bool UGunnerMainMenuWidget::CanStartGame() const
{
	FNamedOnlineSession* Session = SessionInterfacePtr->GetNamedSession(NAME_GameSession);
	return Session && Session->RegisteredPlayers.Num() > 1;
}


void UGunnerMainMenuWidget::JoinSession(FString SessionId)
{
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	OnJoinSessionCompleteDelegateHandle = SessionInterfacePtr->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UGunnerMainMenuWidget::OnJoinSessionComplete));
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
