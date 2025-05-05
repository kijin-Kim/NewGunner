// Copyright Epic Games, Inc. All Rights Reserved.


#include "GunnerPlayerController.h"

#include "Action/NexusActionComponent.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Canvas.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Gunner/_Core/GunnerGameInstance.h"
#include "Gunner/_Core/GunnerTeamAgentInterface.h"
#include "Gunner/_Core/Input/GunnerInputEventDispatcherComponent.h"
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
	DisplayDebugManager.DrawString(FString::Printf(TEXT("ServerTime: %fsec, Ping: %fms, RTT: %fms"),
	                                               GameStateBase ? GameStateBase->GetServerWorldTimeSeconds() : 0.0f,
	                                               PlayerState ? PlayerState->GetPingInMilliseconds() : 0.0f,
	                                               PlayerState ? PlayerState->GetPingInMilliseconds() * 0.5f : 0.0f));
}

void AGunnerPlayerController::InitPlayerState()
{
	Super::InitPlayerState();
	if (IsLocalController())
	{
		TrySetParameterCollectionLocalPlayerTeamID();
	}
}

void AGunnerPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (IsLocalController())
	{
		CreateMainWidget();
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
			TrySetParameterCollectionLocalPlayerTeamID();
			CreateMainWidget();
		}));
	}
}


void AGunnerPlayerController::TrySetParameterCollectionLocalPlayerTeamID()
{
	IGunnerTeamAgentInterface* TeamAgentInterface = Cast<IGunnerTeamAgentInterface>(PlayerState);
	if (ensure(TeamAgentInterface))
	{
		SetParameterCollectionLocalPlayerTeamID(TeamAgentInterface->GetGenericTeamId());

		TeamAgentInterface->GetOnTeamSetDelegate()->AddWeakLambda(this, [this](FGenericTeamId OldTeamId, FGenericTeamId NewTeamId)
		{
			SetParameterCollectionLocalPlayerTeamID(NewTeamId);
		});
	}
}

void AGunnerPlayerController::SetParameterCollectionLocalPlayerTeamID(FGenericTeamId TeamID)
{
	UGunnerGameInstance* GameInstance = GetWorld()->GetGameInstance<UGunnerGameInstance>();
	check(GameInstance->MaterialParameterCollection);
	UMaterialParameterCollectionInstance* MaterialParameterCollectionInstance = GetWorld()->GetParameterCollectionInstance(GameInstance->MaterialParameterCollection);
	MaterialParameterCollectionInstance->SetScalarParameterValue(TEXT("LocalPlayerTeamID"), TeamID);
}

void AGunnerPlayerController::CreateMainWidget()
{
	if (MainWidgetClass)
	{
		MainWidget = CreateWidget<UUserWidget>(this, MainWidgetClass);
		if (MainWidget)
		{
			MainWidget->AddToViewport();
		}
	}
}
