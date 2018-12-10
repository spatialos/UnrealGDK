// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using UnrealBuildTool;

public class SpatialGDKEditorCommandlet : ModuleRules
{
	public SpatialGDKEditorCommandlet(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"SpatialGDKEditor",
				"UnrealEd"
			});

		PrivateIncludePaths.AddRange(
			new string[]
			{
				"Private",
				"Private/Commandlets"
			});
	}
}
