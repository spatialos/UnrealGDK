// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using UnrealBuildTool;

public class SpatialGDKTests : ModuleRules
{
	public SpatialGDKTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
#pragma warning disable 0618
        bFasterWithoutUnity = true; // Deprecated in 4.24, replace with bUseUnity = true; once we drop support for 4.23
#pragma warning restore 0618

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
