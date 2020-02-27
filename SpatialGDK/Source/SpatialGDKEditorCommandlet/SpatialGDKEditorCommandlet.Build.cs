// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using UnrealBuildTool;

public class SpatialGDKEditorCommandlet : ModuleRules
{
	public SpatialGDKEditorCommandlet(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
#pragma warning disable 0618
		bFasterWithoutUnity = true;             // Deprecated in 4.24, replace with bUseUnity = false; once we drop support for 4.23
		if (Target.Version.MinorVersion == 24)  // Due to a bug in 4.24, bFasterWithoutUnity is inversed, fixed in master, so should hopefully roll into the next release, remove this once it does
		{
			bFasterWithoutUnity = false;
		}
#pragma warning restore 0618

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"EngineSettings",
				"SpatialGDK",
				"SpatialGDKEditor",
				"UnrealEd",
			});

		PrivateIncludePaths.AddRange(
			new string[]
			{
				"SpatialGDKEditorCommandlet/Private",
				"SpatialGDKEditorCommandlet/Private/Commandlets"
			});
	}
}
