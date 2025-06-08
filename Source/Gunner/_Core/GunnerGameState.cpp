// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerGameState.h"

#include "Engine/CanvasRenderTarget2D.h"
#include "GameFramework/PlayerState.h"
#include "Gunner/Gunner.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "OnlineSession/GunnerSessionHelperSubsystem.h"

FString FGunnerKillFeed::ToString() const
{
	return FString::Printf(TEXT("KillLog={Killer: %s, Victim: %s, Cause: %s}"), *KillerPlayerState->GetPlayerName(), *VictimPlayerState->GetPlayerName(), *KillCauserName.ToString());
}

AGunnerGameState::AGunnerGameState()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGunnerGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGunnerGameState, KillInfos);
	DOREPLIFETIME(AGunnerGameState, ServerMatchTimeLimitSeconds);
}

void AGunnerGameState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	for (auto& [PlayerID, RenderTargets] : PlayerFogOfWarRenderTargets)
	{
		if (RenderTargets.VisionConeRenderTarget)
		{
			RenderTargets.VisionConeRenderTarget->UpdateResource();
		}

		if (RenderTargets.InformationRenderTarget)
		{
			RenderTargets.InformationRenderTarget->UpdateResource();
		}
	}
}

void AGunnerGameState::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	UE_LOG(LogGunner, Log, TEXT("Match Has Started: PlayerNum: %d"), PlayerArray.Num());
}

void AGunnerGameState::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();
	KillInfos.Empty();
}

void AGunnerGameState::OnRep_KillInfos()
{
}

void AGunnerGameState::UpdateKillInfos(AController* Killer)
{
	FGunnerKillInfo* KillerInfo = KillInfos.FindByPredicate([Killer](const FGunnerKillInfo& Info)
	{
		return Info.KillerPlayerId == Killer->PlayerState->GetPlayerId();
	});

	if (KillerInfo)
	{
		KillerInfo->Kills++;
		return;
	}

	FGunnerKillInfo NewInfo;
	IGenericTeamAgentInterface* TeamAgentInterface = Cast<IGenericTeamAgentInterface>(Killer->PlayerState);
	if (ensure(TeamAgentInterface))
	{
		NewInfo.TeamId = TeamAgentInterface->GetGenericTeamId();
	}
	NewInfo.KillerPlayerId = Killer->PlayerState->GetPlayerId();
	NewInfo.Kills = 1;
	KillInfos.Add(NewInfo);
}

const FGunnerFogOfWarRenderTargets& AGunnerGameState::FindOrAddPlayerFogOfWarRenderTargets(int32 PlayerID)
{
	if (!PlayerFogOfWarRenderTargets.Contains(PlayerID))
	{
		FGunnerFogOfWarRenderTargets NewRenderTargets;
		NewRenderTargets.VisionConeRenderTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(
			GetWorld(), UCanvasRenderTarget2D::StaticClass(), 1024, 1024);
		NewRenderTargets.InformationRenderTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(
			GetWorld(), UCanvasRenderTarget2D::StaticClass(), 1024, 1024);
		PlayerFogOfWarRenderTargets.Add(PlayerID, NewRenderTargets);
	}

	return PlayerFogOfWarRenderTargets[PlayerID];
}

void AGunnerGameState::NetMulticastBroadcastKill_Implementation(const FGunnerKillFeed& KillLog)
{
	OnNewKillConfirmedDelegate.Broadcast(KillLog);
}

void AGunnerGameState::NetMulticastBroadcastWinners_Implementation(const TArray<int32>& WinnerIds)
{
	OnMatchEndedDelegate.Broadcast(WinnerIds);

	if (HasAuthority())
	{
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			UGunnerSessionHelperSubsystem* SessionHelperSubsystem = GetGameInstance()->GetSubsystem<UGunnerSessionHelperSubsystem>();
			check(SessionHelperSubsystem);
			SessionHelperSubsystem->DestroySession();
			UGameplayStatics::OpenLevel(GetWorld(), TEXT("/Game/Maps/MainMenu"));
		}, 5.f * UGameplayStatics::GetGlobalTimeDilation(GetWorld()), false);
	}
}
