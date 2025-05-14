// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerGameInstance.h"

#include "MoviePlayer.h"
#include "Blueprint/UserWidget.h"

#include "Prediction/NexusPrediction.h"

void UGunnerGameInstance::Init()
{
	Super::Init();

	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UGunnerGameInstance::PostLoadMapWithWorld);

	FWorldDelegates::OnSeamlessTravelStart.RemoveAll(this);
	FWorldDelegates::OnSeamlessTravelStart.AddUObject(this, &UGunnerGameInstance::OnSeamlessTravelStart);
}

void UGunnerGameInstance::Shutdown()
{
	Super::Shutdown();
	FNexusPredictionEvents::ResetPredictionEvents();
}

void UGunnerGameInstance::OnSeamlessTravelStart(UWorld* World, const FString& MapName)
{
	PlayLoadingScreen();
}


void UGunnerGameInstance::PostLoadMapWithWorld(UWorld* InLoadedWorld)
{
	//StopLoadingScreen();
}

void UGunnerGameInstance::PlayLoadingScreen()
{
	if (!LoadingScreenWidgetClass)
	{
		return;
	}

	UUserWidget* LoadingScreenWidget = CreateWidget<UUserWidget>(this, LoadingScreenWidgetClass);
	
	FLoadingScreenAttributes LoadingScreenAttributes;
	LoadingScreenAttributes.WidgetLoadingScreen = LoadingScreenWidget->TakeWidget();
	LoadingScreenAttributes.MinimumLoadingScreenDisplayTime = 3.0f;
	LoadingScreenAttributes.bAutoCompleteWhenLoadingCompletes = false;
	LoadingScreenAttributes.bMoviesAreSkippable = false;
	LoadingScreenAttributes.bWaitForManualStop = true;
	LoadingScreenAttributes.bAllowEngineTick = false;
	GetMoviePlayer()->SetupLoadingScreen(LoadingScreenAttributes);
	GetMoviePlayer()->PlayMovie();
}

void UGunnerGameInstance::StopLoadingScreen()
{
	GetMoviePlayer()->StopMovie();
}
