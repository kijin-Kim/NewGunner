// Copyright Epic Games, Inc. All Rights Reserved.


#include "GunnerPlayerController.h"

#include "Action/NexusActionComponent.h"
#include "Engine/Canvas.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerState.h"
#include "Gunner/_Core/GunnerGameInstance.h"
#include "Gunner/_Core/GunnerTeamAgentInterface.h"
#include "Gunner/_Core/Input/GunnerInputEventDispatcherComponent.h"
#include "Gunner/_Core/UI/GunnerHUD.h"
#include "Materials/MaterialParameterCollectionInstance.h"


AGunnerPlayerController::AGunnerPlayerController()
{
	InputEventDispatcherComponent = CreateDefaultSubobject<UGunnerInputEventDispatcherComponent>(TEXT("InputEventDispatcher"));
	bAutoManageActiveCameraTarget = false;
}

void AGunnerPlayerController::DisplayDebug(class UCanvas* Canvas, const class FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;
	AGameStateBase* GameStateBase = GetWorld()->GetGameState();
	DisplayDebugManager.SetDrawColor(FColor::White);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("ServerTime: %fsec"), GameStateBase ? GameStateBase->GetServerWorldTimeSeconds() : 0.0f));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("Ping: %fms"), PlayerState ? PlayerState->GetPingInMilliseconds() : 0.0f));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("RTT: %fms"), PlayerState ? PlayerState->GetPingInMilliseconds() * 0.5f : 0.0f));
}

void AGunnerPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (IsLocalController())
	{
		AGunnerHUD* GunnerHUD = Cast<AGunnerHUD>(GetHUD());
		check(GunnerHUD)
		GunnerHUD->SetupHUD(PlayerState);
	}
}

void AGunnerPlayerController::InitPlayerState()
{
	Super::InitPlayerState();
	if (IsLocalController())
	{
		TrySetParameterCollectionLocalPlayerTeamID();
	}
}

void AGunnerPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	if (PlayerState)
	{
		UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(PlayerState);
		check(ActionComponent);
		ActionComponent->CallOrAddSetupCompletedDelegate(FOnNexusActionComponentSetupCompletedSignature::FDelegate::CreateWeakLambda(this, [this]()
		{
			AGunnerHUD* GunnerHUD = Cast<AGunnerHUD>(GetHUD());
			check(GunnerHUD)
			GunnerHUD->SetupHUD(PlayerState);
			TrySetParameterCollectionLocalPlayerTeamID();
		}));		
	}

}


void AGunnerPlayerController::TrySetParameterCollectionLocalPlayerTeamID()
{
	IGunnerTeamAgentInterface* TeamAgentInterface = Cast<IGunnerTeamAgentInterface>(PlayerState);
	check(TeamAgentInterface);
	SetParameterCollectionLocalPlayerTeamID(TeamAgentInterface->GetGenericTeamId());

	TeamAgentInterface->GetOnTeamSetDelegate()->AddWeakLambda(this, [this](FGenericTeamId OldTeamId, FGenericTeamId NewTeamId)
	{
		SetParameterCollectionLocalPlayerTeamID(NewTeamId);
	});
}

void AGunnerPlayerController::SetParameterCollectionLocalPlayerTeamID(FGenericTeamId TeamID)
{
	UGunnerGameInstance* GameInstance = GetWorld()->GetGameInstance<UGunnerGameInstance>();
	check(GameInstance->MaterialParameterCollection);
	UMaterialParameterCollectionInstance* MaterialParameterCollectionInstance = GetWorld()->GetParameterCollectionInstance(GameInstance->MaterialParameterCollection);
	MaterialParameterCollectionInstance->SetScalarParameterValue(TEXT("LocalPlayerTeamID"), TeamID);
}
