// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Uiana : ModuleRules
{
	public Uiana(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"InputCore",
				"ToolMenus",
				"CoreUObject",
				"Engine",
				"Settings",
				"Slate",
				"RenderCore",
                "WorkspaceMenuStructure",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		if(Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"UnrealEd",
					"PythonScriptPlugin",
					"EditorFramework",
				}
				);
		}
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
