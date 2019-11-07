// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using UnrealBuildTool;

public class SpatialGDKServices : ModuleRules
{
	public SpatialGDKServices(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bFasterWithoutUnity = true;

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
