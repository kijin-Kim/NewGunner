// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerViewModel_TeamDeathMatch.h"

#include "Blueprint/UserWidget.h"
#include "Gunner/_Core/GunnerTeamDeathMatchGameState.h"
#include "GameFramework/PlayerState.h"


UObject* UGunnerViewModelContextResolver::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const
{
	return UGunnerViewModel_TeamDeathMatch::CreateInstance(ExpectedType, UserWidget, View);
}

void UGunnerViewModelContextResolver::DestroyInstance(const UObject* ViewModel, const UMVVMView* View) const
{
	Super::DestroyInstance(ViewModel, View);
	CastChecked<UGunnerViewModel_TeamDeathMatch>(ViewModel)->OnDestroyViewModel(ViewModel, View);
}

UGunnerViewModel_TeamDeathMatch* UGunnerViewModel_TeamDeathMatch::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View)
{
	UGunnerViewModel_TeamDeathMatch* ViewModel = NewObject<UGunnerViewModel_TeamDeathMatch>(UserWidget->GetOwningPlayer(), ExpectedType);
	check(ViewModel);
	ViewModel->OnCreateViewModel(UserWidget);
	return ViewModel;
}

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
