// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerDeathMatchGameState.h"

#include "GunnerOverlayWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetStringLibrary.h"
#include "UI/GunnerHUD.h"

AGunnerDeathMatchGameState::AGunnerDeathMatchGameState()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGunnerDeathMatchGameState::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	GunnerHUD = Cast<AGunnerHUD>(UGameplayStatics::GetActorOfClass(GetWorld(), AGunnerHUD::StaticClass()));
	check(GunnerHUD);
}

void AGunnerDeathMatchGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (MatchState != MatchState::InProgress)
	{
		return;
	}
	if (UGunnerOverlayWidget* OverlayWidget = GunnerHUD->OverlayWidget)
	{
		double RemainingTime = GetMatchTimeLimit() - GetServerWorldTimeSeconds();
		OverlayWidget->TimeText->SetText(FText::FromString(UKismetStringLibrary::TimeSecondsToString(RemainingTime)));
	}
}

TArray<int32> AGunnerDeathMatchGameState::DetermineWinners() const
{
	int32 MaxKills = 0;
	int32 WinnerID = 0;
	for (const FGunnerPlayerKillInfo& Info : PlayerKills)
	{
		if (Info.Kills > MaxKills)
		{
			MaxKills = Info.Kills;
			WinnerID = Info.PlayerId;
		}
	}
	return {WinnerID};
}
