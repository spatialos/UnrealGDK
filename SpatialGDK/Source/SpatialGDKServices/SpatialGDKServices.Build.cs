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
				"Core",
				"Engine",
				"SpatialGDK",
            });

        PublicDependencyModuleNames.Add("SpatialGDKEditorToolbar");
        PrivateIncludePaths.Add("SpatialGDKServices/Private");
	}
}
