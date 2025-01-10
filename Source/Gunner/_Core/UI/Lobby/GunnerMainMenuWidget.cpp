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
	// HostButton->Button->OnClicked.AddDynamic(this, &UGunnerMainMenuWidget::OnHostButtonClicked);
	// JoinButton->Button->OnClicked.AddDynamic(this, &UGunnerMainMenuWidget::OnJoinButtonClicked);
	// DeveloperToolButton->Button->OnClicked.AddDynamic(this, &UGunnerMainMenuWidget::OnDeveloperToolButtonClicked);
	// ShutdownButton->Button->OnClicked.AddDynamic(this, &UGunnerMainMenuWidget::OnShutdownButtonClicked);
	//
	// LoopbackJoinButton->Button->OnClicked.AddDynamic(this, &UGunnerMainMenuWidget::OnLoopbackJoinButtonClicked);
	// LoopbackHostButton->Button->OnClicked.AddDynamic(this, &UGunnerMainMenuWidget::OnLoopbackHostButtonClicked);
	// BackwardButton->Button->OnClicked.AddDynamic(this, &UGunnerMainMenuWidget::OnBackwardButtonClicked);
	//
	// QuitMenuButton->Button->OnClicked.AddDynamic(this, &UGunnerMainMenuWidget::OnQuitMenuButtonClicked);
}

void UGunnerMainMenuWidget::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	SessionInterfacePtr->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
	if (bWasSuccessful)
	{
		UE_LOG(LogGunner, Display, TEXT("Session created: %s"), *SessionName.ToString());
		GetWorld()->ServerTravel("/Game/Maps/FirstPersonMap?listen");
	}
	else
	{
		UE_LOG(LogGunner, Error, TEXT("Session creation failed"));
	}
}

void UGunnerMainMenuWidget::OnFindSessionsComplete(bool bWasSuccessful)
{
	SessionInterfacePtr->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);
	if (bWasSuccessful && SessionSearch.IsValid())
	{
		UE_LOG(LogGunner, Display, TEXT("Session found"));
		const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
		OnJoinSessionCompleteDelegateHandle = SessionInterfacePtr->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UGunnerMainMenuWidget::OnJoinSessionComplete));
		if (!SessionInterfacePtr->JoinSession(LocalPlayer->GetControllerId(), NAME_GameSession, SessionSearch->SearchResults[0]))
		{
			SessionInterfacePtr->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
			OnJoinSessionComplete(NAME_None, EOnJoinSessionCompleteResult::UnknownError);
		}
	}
	else
	{
		UE_LOG(LogGunner, Error, TEXT("Session not found"));
	}
}

void UGunnerMainMenuWidget::OnJoinSessionComplete(FName Name, EOnJoinSessionCompleteResult::Type Arg)
{
	SessionInterfacePtr->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
	if (Arg == EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(LogGunner, Display, TEXT("Session joined"));
		FString Address;
		SessionInterfacePtr->GetResolvedConnectString(NAME_GameSession, Address);
		GetOwningPlayer()->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
	}
	else
	{
		UE_LOG(LogGunner, Error, TEXT("Session join failed"));
	}
}

void UGunnerMainMenuWidget::OnJoinButtonClicked()
{
	// LastWidgetIndex = WidgetSwitcher->GetActiveWidgetIndex();
	// WidgetSwitcher->SetActiveWidgetIndex(2);


	if (GIsPlayInEditorWorld)
	{
		UE_LOG(LogGunner, Error, TEXT("Cannot join in editor world"));
		return;
	}


	// Join Session
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

void UGunnerMainMenuWidget::OnHostButtonClicked()
{
	if (GIsPlayInEditorWorld)
	{
		UE_LOG(LogGunner, Error, TEXT("Cannot host in editor world"));
		return;
	}

	if (SessionInterfacePtr->GetNamedSession(NAME_GameSession))
	{
		SessionInterfacePtr->DestroySession(NAME_GameSession);
	}
	OnCreateSessionCompleteDelegateHandle = SessionInterfacePtr->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &UGunnerMainMenuWidget::OnCreateSessionComplete));

	FOnlineSessionSettings NewSessionSettings;
	NewSessionSettings.bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	NewSessionSettings.bUseLobbiesIfAvailable = true;
	NewSessionSettings.bShouldAdvertise = true;
	NewSessionSettings.bUsesPresence = true;
	NewSessionSettings.NumPublicConnections = 2;
	NewSessionSettings.BuildUniqueId = 1;

	if (!SessionInterfacePtr->CreateSession(0, NAME_GameSession, NewSessionSettings))
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
