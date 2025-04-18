using UnrealBuildTool;

public class GunnerTests : ModuleRules
{
	public GunnerTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(new string[] { "Core",
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"UnrealEd",
			"Gunner", 
			"NexusAction",
			"GameplayTags"
		});

		PrivateIncludePathModuleNames.Add("AutomationController");

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("EditorSubsystem");
		}
	}
}