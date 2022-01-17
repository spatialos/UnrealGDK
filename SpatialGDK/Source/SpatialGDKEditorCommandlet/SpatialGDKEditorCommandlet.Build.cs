// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using UnrealBuildTool;

public class SpatialGDKEditorCommandlet : ModuleRules
{
	public SpatialGDKEditorCommandlet(ReadOnlyTargetRules Target) : base(Target)
	{
		bLegacyPublicIncludePaths = false;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		string unity_mode_env = System.Environment.GetEnvironmentVariable("UNITY_MODE");
		bool unity_mode = bool.Parse(unity_mode_env);
		bUseUnity = unity_mode;

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"EngineSettings",
				"SpatialGDK",
				"SpatialGDKEditor",
				"SpatialGDKServices",
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
