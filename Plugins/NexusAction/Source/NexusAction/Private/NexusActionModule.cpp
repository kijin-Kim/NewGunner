// Copyright Epic Games, Inc. All Rights Reserved.

#include "NexusActionModule.h"

#include "Action/NexusActionComponent.h"
#include "GameFramework/HUD.h"

#define LOCTEXT_NAMESPACE "FNexusActionModule"

void FNexusActionModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	AHUD::OnShowDebugInfo.AddStatic(&UNexusActionComponent::OnShowDebugInfo);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FNexusActionModule, NexusAction)
