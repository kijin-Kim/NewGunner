// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Gunner : ModuleRules
{
	public Gunner(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		if(Target.bBuildEditor)
		{
			PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "Slate", "SlateCore", "GameplayTags", "UnrealPSKPSA", "OnlineSubsystem", "OnlineSubsystemSteam", "Niagara" });
		}
		else
		{
			PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG","Slate", "SlateCore", "GameplayTags", "OnlineSubsystem", "OnlineSubsystemSteam" });
		}
		PrivateDependencyModuleNames.AddRange(new string[] { "NetCore" });
	}
}
