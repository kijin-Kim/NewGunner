using UnrealBuildTool;

public class NexusActionTests : ModuleRules
{
	public NexusActionTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(new string[] { "NexusAction" });
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"UnrealEd",
			"NexusAction"
		});

		PrivateIncludePathModuleNames.Add("AutomationController");

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("EditorSubsystem");
		}
	}
}