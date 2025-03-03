// Copyright Epic Games, Inc. All Rights Reserved.

#include "NexusActionModule.h"
#include "NexusPrediction.h"

#define LOCTEXT_NAMESPACE "FNexusActionModule"

void FNexusActionModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FNexusActionModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FNexusPredictionEvents::Clear();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FNexusActionModule, NexusAction)