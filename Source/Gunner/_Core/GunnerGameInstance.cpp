// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerGameInstance.h"

#include "MoviePlayer.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Prediction/NexusPrediction.h"


void UGunnerGameInstance::Init()
{
	Super::Init();
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

void UGunnerGameInstance::PlayLoadingScreen()
{
	if (LoadingScreenWidgetClass)
	{
		LoadingScreenWidget = CreateWidget<UUserWidget>(this, LoadingScreenWidgetClass);
		FLoadingScreenAttributes LoadingScreenAttributes;
		LoadingScreenAttributes.WidgetLoadingScreen = LoadingScreenWidget->TakeWidget();
		LoadingScreenAttributes.MinimumLoadingScreenDisplayTime = 5.0f;
		LoadingScreenAttributes.bAutoCompleteWhenLoadingCompletes = true;
		LoadingScreenAttributes.bAllowEngineTick = true;
		LoadingScreenAttributes.bAllowInEarlyStartup = false;

		GetMoviePlayer()->SetupLoadingScreen(LoadingScreenAttributes);
		GetMoviePlayer()->PlayMovie();
	}
}