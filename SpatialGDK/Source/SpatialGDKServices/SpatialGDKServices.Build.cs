// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using UnrealBuildTool;

public class SpatialGDKServices : ModuleRules
{
	public SpatialGDKServices(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
#pragma warning disable 0618
        bFasterWithoutUnity = true; // Deprecated in 4.24, replace with bUseUnity = true; once we drop support for 4.23
#pragma warning restore 0618

        PrivateDependencyModuleNames.AddRange(
			new string[] {
				"EditorStyle",
				"Engine",
				"OutputLog",
				"Slate",
				"SlateCore",
				"Core",
				"CoreUObject",
				"EngineSettings",
				"Json",
				"JsonUtilities",
				"UnrealEd",
				"Sockets"
			}
		);
	}
}
