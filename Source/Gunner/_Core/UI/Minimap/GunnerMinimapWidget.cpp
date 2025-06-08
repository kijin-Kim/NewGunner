// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerMinimapWidget.h"

#include "Components/Image.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "GameFramework/PlayerState.h"
#include "Gunner/_Core/GunnerFogOfWarData.h"
#include "Gunner/_Core/GunnerGameState.h"
#include "Gunner/_Core/GunnerWorldSettings.h"

void UGunnerMinimapWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	if (ensure(MinimapMaterial))
	{
		MinimapDynamicMaterialInstance = UMaterialInstanceDynamic::Create(MinimapMaterial, this);
	}

	if (ensure(MinimapImage) && ensure(MinimapDynamicMaterialInstance))
	{
		MinimapImage->SetBrushFromMaterial(MinimapDynamicMaterialInstance);
	}
}

void UGunnerMinimapWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!MinimapDynamicMaterialInstance)
	{
		return;
	}

	AGunnerGameState* GunnerGameState = GetWorld()->GetGameState<AGunnerGameState>();
	IGunnerTeamAgentInterface* TeamAgentInterface = GetOwningPlayer()->GetPlayerState<IGunnerTeamAgentInterface>();
	if (!GunnerGameState || !TeamAgentInterface)
	{
		return;
	}

	const FGunnerFogOfWarRenderTargets& FogOfWarRenderTargets = GunnerGameState->FindOrAddPlayerFogOfWarRenderTargets(TeamAgentInterface->GetGenericTeamId().GetId());
	if (FogOfWarRenderTargets.VisionConeRenderTarget)
	{
		MinimapDynamicMaterialInstance->SetTextureParameterValue(TEXT("FogRenderTarget"), FogOfWarRenderTargets.VisionConeRenderTarget);
		MinimapDynamicMaterialInstance->SetTextureParameterValue(TEXT("InformationRenderTarget"), FogOfWarRenderTargets.InformationRenderTarget);
	}

	AGunnerWorldSettings* WorldSettings = Cast<AGunnerWorldSettings>(GetWorld()->GetWorldSettings());
	if (!WorldSettings)
	{
		return;
	}

	UGunnerFogOfWarData* FogOfWarData = WorldSettings->GetFogOfWarData();
	if (!FogOfWarData)
	{
		return;
	}

	if (FogOfWarData->FogTexture)
	{
		MinimapDynamicMaterialInstance->SetTextureParameterValue(TEXT("FogTexture"), FogOfWarData->FogTexture);
	}

	if (FogOfWarData->RevealedTexture)
	{
		MinimapDynamicMaterialInstance->SetTextureParameterValue(TEXT("RevealedTexture"), FogOfWarData->RevealedTexture);
	}
}
