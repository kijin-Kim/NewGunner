// Copyright Epic Games, Inc. All Rights Reserved.


#include "GunnerPlayerController.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/HUD.h"
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
	AGunnerHUD* GunnerHUD = Cast<AGunnerHUD>(GetHUD());
	check(GunnerHUD)
	GunnerHUD->SetupHUD(PlayerState);

	TrySetParameterCollectionLocalPlayerTeamID();
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
