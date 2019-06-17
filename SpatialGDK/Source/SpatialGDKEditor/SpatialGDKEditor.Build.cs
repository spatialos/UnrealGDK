// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using UnrealBuildTool;

public class SpatialGDKEditor : ModuleRules
{
	public SpatialGDKEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bFasterWithoutUnity = true;

        PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
 				"EngineSettings",
                "PropertyEditor",
                "Slate",
                "SlateCore",
                "SpatialGDK",
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
