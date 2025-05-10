// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerGameState.h"

#include "GameFramework/PlayerState.h"
#include "Gunner/Gunner.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

FString FGunnerKillLog::ToString() const
{
	return FString::Printf(TEXT("KillLog={Killer: %s, Victim: %s, Cause: %s}"), *KillerPlayerState->GetPlayerName(), *VictimPlayerState->GetPlayerName(), *KillCauserName.ToString());
}

void AGunnerGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGunnerGameState, ServerMatchTimeLimitSeconds);
	DOREPLIFETIME(AGunnerGameState, KillInfos);
}

FGunnerKillInfo* AGunnerGameState::GetKillerInfo(AController* Killer)
{
	return KillInfos.FindByPredicate([Killer](const FGunnerKillInfo& Info)
	{
		return Info.KillerPlayerId == Killer->PlayerState->GetPlayerId();
	});
}


void AGunnerGameState::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();
	NetMulticastBroadcastWinners(DetermineWinners());
}

void AGunnerGameState::SetMatchTimeLimitSeconds(double TimeLimitSeconds)
{
	ServerMatchTimeLimitSeconds = TimeLimitSeconds;
}

void AGunnerGameState::NetMulticastBroadcastKill_Implementation(const FGunnerKillLog& KillLog)
{
	OnNewKillConfirmedDelegate.Broadcast(KillLog);
}

void AGunnerGameState::NetMulticastBroadcastWinners_Implementation(const TArray<int32>& WinnerIds)
{
	OnMatchEndedDelegate.Broadcast(WinnerIds);
	FTimerHandle TimerHandle;

	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			UGameplayStatics::OpenLevel(GetWorld(), TEXT("/Game/Maps/MainMenu"));
		}, 5.f * UGameplayStatics::GetGlobalTimeDilation(GetWorld()), false);
	}
}

void AGunnerGameState::AuthRegisterKill(AController* Killer, AController* Victim, FName KillCauserName)
{
	if (!HasAuthority())
	{
		GR_LOG(LogGunner, Error, TEXT("권한 없는 함수 호출"));
		return;
	}

	if (!Killer || !Victim)
	{
		return;
	}

	FGunnerKillLog KillLog;
	KillLog.KillerPlayerState = Killer->PlayerState;
	KillLog.VictimPlayerState = Victim->PlayerState;
	KillLog.KillCauserName = KillCauserName;
	NetMulticastBroadcastKill(KillLog);

	if (FGunnerKillInfo* KillerInfo = GetKillerInfo(Killer))
	{
		KillerInfo->Kills++;
		return;
	}

	FGunnerKillInfo NewInfo;
	NewInfo.KillerPlayerId = Killer->PlayerState->GetPlayerId();
	NewInfo.Kills = 1;
	KillInfos.Add(NewInfo);
}
