// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using UnrealBuildTool;

public class SpatialGDKEditor : ModuleRules
{
	public SpatialGDKEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
#pragma warning disable 0618
        bFasterWithoutUnity = true; // Deprecated in 4.24, replace with bUseUnity = true; once we drop support for 4.23
#pragma warning restore 0618

        PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"EditorStyle",
				"Engine",
 				"EngineSettings",
 				"IOSRuntimeSettings",
 				"Json",
				"PropertyEditor",
				"Slate",
				"SlateCore",
				"SpatialGDK",
				"SpatialGDKServices",
				"UnrealEd",
				"GameplayAbilities"
			});

		PrivateIncludePaths.AddRange(
			new string[]
			{
				"SpatialGDKEditor/Private",
			});
	}
}
