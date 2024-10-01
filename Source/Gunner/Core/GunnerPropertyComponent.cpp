// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerPropertyComponent.h"
#include "DisplayDebugHelpers.h"
#include "Engine/Canvas.h"


void UGunnerPropertyComponent::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y)
{
	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;
	DisplayDebugManager.SetDrawColor(FColor::Orange);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("Property: %s"), *GetName()));
}
