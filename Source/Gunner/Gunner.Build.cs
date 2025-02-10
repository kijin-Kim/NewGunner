// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Gunner : ModuleRules
{
	public Gunner(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		if(Target.bBuildEditor)
		{
			PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "Slate", "SlateCore", "GameplayTags", "UnrealPSKPSA", "OnlineSubsystem", "OnlineSubsystemSteam", "OnlineSubsystemNull","Niagara", "AIModule" });
		}
		else
		{
			PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG","Slate", "SlateCore", "GameplayTags", "OnlineSubsystem", "OnlineSubsystemSteam", "OnlineSubsystemNull", "AIModule" });
		}
		PrivateDependencyModuleNames.AddRange(new string[] { "NetCore", "MoviePlayer" });
	}
}
