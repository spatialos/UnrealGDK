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
				"EditorStyle",
				"Engine",
 				"EngineSettings",
				"Json",
				"PropertyEditor",
				"Slate",
				"SlateCore",
				"SpatialGDK",
				"SpatialGDKServices",
				"UnrealEd",
				"GameplayAbilities",
                "SessionFrontend",
                "TcpMessaging",
                "EngineMessages"
            });

		PrivateIncludePaths.AddRange(
			new string[]
			{
				"SpatialGDKEditor/Private",
			});
	}
}
