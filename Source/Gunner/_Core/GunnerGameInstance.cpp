// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerGameInstance.h"

#include "GunnerGameMode.h"
#include "MoviePlayer.h"
#include "Prediction/NexusPrediction.h"
#include "Blueprint/UserWidget.h"
#include "Cheat/GunnerCheatManager.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "Gunner/Gunner.h"
#include "Gunner/Item/GunnerInventoryManagerComponent.h"

void UGunnerGameInstance::Init()
{
	Super::Init();

	static bool bIsInitialized = false;
	if (!bIsInitialized)
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
		FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UGunnerGameInstance::PostLoadMapWithWorld);

		FWorldDelegates::OnSeamlessTravelStart.RemoveAll(this);
		FWorldDelegates::OnSeamlessTravelStart.AddUObject(this, &UGunnerGameInstance::OnSeamlessTravelStart);

		AHUD::OnShowDebugInfo.RemoveAll(this);
		AHUD::OnShowDebugInfo.AddUObject(this, &UGunnerGameInstance::OnShowDebugInfo);

		FGameModeEvents::GameModePostLoginEvent.AddUObject(this, &UGunnerGameInstance::OnPlayerPostLogin);


		bIsInitialized = true;
	}
}

void UGunnerGameInstance::Shutdown()
{
	Super::Shutdown();
	FNexusPredictionEvents::ResetPredictionEvents();
}

UCurveTable* UGunnerGameInstance::GetDamageCurveTable() const
{
	return DamageCurveTable;
}

UDataTable* UGunnerGameInstance::GetWeaponDataTable() const
{
	return WeaponDataTable;
}


void UGunnerGameInstance::OnSeamlessTravelStart(UWorld* World, const FString& MapName)
{
	PlayLoadingScreen();
}

void UGunnerGameInstance::PostLoadMapWithWorld(UWorld* InLoadedWorld)
{
	StopLoadingScreen();
}

void UGunnerGameInstance::OnShowDebugInfo(AHUD* Ahud, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Arg)
{
	UGunnerInventoryManagerComponent::OnShowDebugInfo(Ahud, Canvas, DebugDisplayInfo, X, Arg);
}

void UGunnerGameInstance::PlayLoadingScreen()
{
	if (!IsValid(LoadingScreenWidgetClass))
	{
		return;
	}

	UUserWidget* LoadingScreenWidget = CreateWidget<UUserWidget>(this, LoadingScreenWidgetClass);
	TSharedRef<SWidget> LoadingScreenWidgetRef = LoadingScreenWidget->TakeWidget();

	FLoadingScreenAttributes LoadingScreenAttributes;
	LoadingScreenAttributes.WidgetLoadingScreen = LoadingScreenWidgetRef;
	// LoadingScreenAttributes.WidgetLoadingScreen = FLoadingScreenAttributes::NewTestLoadingScreenWidget();
	LoadingScreenAttributes.MinimumLoadingScreenDisplayTime = 3.0f;
	LoadingScreenAttributes.bAutoCompleteWhenLoadingCompletes = true;
	// LoadingScreenAttributes.bMoviesAreSkippable = false;
	LoadingScreenAttributes.bWaitForManualStop = true;
	// LoadingScreenAttributes.PlaybackType = EMoviePlaybackType::MT_Looped;
	// LoadingScreenAttributes.bAllowInEarlyStartup = false;
	// /** If true, this will call the engine tick while the game thread is stalled waiting for a loading movie to finish. This only works for post-startup load screens and is potentially unsafe */
	LoadingScreenAttributes.bAllowEngineTick = true;
	GetMoviePlayer()->SetupLoadingScreen(LoadingScreenAttributes);
	GetMoviePlayer()->PlayMovie();
}

void UGunnerGameInstance::StopLoadingScreen()
{
	GetMoviePlayer()->StopMovie();
}

void UGunnerGameInstance::OnPlayerPostLogin(AGameModeBase* GameModeBase, APlayerController* PlayerController)
{
	if (!PlayerController || !PlayerController->HasAuthority())
	{
		return;
	}

	if (UGunnerCheatManager* CheatManager = CastChecked<UGunnerCheatManager>(PlayerController->CheatManager))
	{
		CheatManager->OnPlayerPostLogin(GameModeBase, PlayerController);
	}
}
