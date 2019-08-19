// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using UnrealBuildTool;

public class SpatialGDKEditorCommandlet : ModuleRules
{
	public SpatialGDKEditorCommandlet(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bFasterWithoutUnity = true;

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"SpatialGDKEditor",
				"UnrealEd",
				"Core",
				"CoreUObject",
				"Engine",
				"SpatialGDK",
			});

		PrivateIncludePaths.AddRange(
			new string[]
			{
				"SpatialGDKEditorCommandlet/Private",
				"SpatialGDKEditorCommandlet/Private/Commandlets"
			});
	}
}
