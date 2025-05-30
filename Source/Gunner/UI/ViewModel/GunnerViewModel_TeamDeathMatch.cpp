// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerViewModel_TeamDeathMatch.h"

#include "Blueprint/UserWidget.h"
#include "Gunner/_Core/GunnerTeamDeathMatchGameState.h"
#include "GameFramework/PlayerState.h"


void UGunnerViewModel_TeamDeathMatch::OnCreateViewModel(const UUserWidget* UserWidget)
{
	APlayerState* OwningPlayerState = UserWidget->GetOwningPlayerState();
	IGenericTeamAgentInterface* TeamAgentInterface = Cast<IGenericTeamAgentInterface>(OwningPlayerState);
	check(TeamAgentInterface);
	OwnerTeamID = TeamAgentInterface->GetGenericTeamId();

	AGunnerTeamDeathMatchGameState* TdmGameState = UserWidget->GetWorld()->GetGameState<AGunnerTeamDeathMatchGameState>();
	if (ensure(TdmGameState))
	{
		TdmGameState->OnTeamKillCountChanged.AddUObject(this, &UGunnerViewModel_TeamDeathMatch::OnTeamKillCountChanged);
	}

	for (const auto& [TeamID, Count] : TdmGameState->GetKillCountPerTeam())
	{
		OnTeamKillCountChanged(TeamID, Count);
	}

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(AllyTeamKillCountText);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(EnemyTeamKillCountText);
}

void UGunnerViewModel_TeamDeathMatch::OnDestroyViewModel(const UObject* Object, const UMVVMView* View) const
{
	AGunnerTeamDeathMatchGameState* TdmGameState = Object->GetWorld()->GetGameState<AGunnerTeamDeathMatchGameState>();
	if (ensure(TdmGameState))
	{
		TdmGameState->OnTeamKillCountChanged.RemoveAll(this);
	}
}

void UGunnerViewModel_TeamDeathMatch::OnTeamKillCountChanged(FGenericTeamId TeamID, int Count)
{
	if (TeamID == OwnerTeamID)
	{
		AllyTeamKillCountText = FText::AsNumber(Count);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(AllyTeamKillCountText);
	}
	else if (TeamID == (OwnerTeamID == AttackerTeam ? DefenderTeam : AttackerTeam))
	{
		EnemyTeamKillCountText = FText::AsNumber(Count);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(EnemyTeamKillCountText);
	}
}
