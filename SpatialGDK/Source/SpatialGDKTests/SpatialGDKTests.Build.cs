// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using UnrealBuildTool;

public class SpatialGDKTests : ModuleRules
{
	public SpatialGDKTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bFasterWithoutUnity = true;

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"SpatialGDK",
				"SpatialGDKEditor",
				"SpatialGDKServices",
				"Core",
				"CoreUObject",
				"Engine",
				"EngineSettings",
				"UnrealEd",
				"Json",
				"JsonUtilities"
			});
	}
}
