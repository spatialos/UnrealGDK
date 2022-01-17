// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using UnrealBuildTool;

public class SpatialGDKEditor : ModuleRules
{
	public SpatialGDKEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		bLegacyPublicIncludePaths = false;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		string unity_mode_env = Environment.GetEnvironmentVariable("UNITY_MODE");
		bool unity_mode = bool.Parse(unity_mode_env);
		bUseUnity = unity_mode;

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"DesktopPlatform",
				"EditorStyle",
				"Engine",
				"EngineSettings",
				"FunctionalTesting",
				"IOSRuntimeSettings",
				"LauncherServices",
				"Json",
				"PropertyEditor",
				"Slate",
				"SlateCore",
				"SpatialGDK",
				"SpatialGDKServices",
				"UATHelper",
				"UnrealEd",
				"DesktopPlatform",
				"MessageLog",
			});

		PrivateIncludePaths.AddRange(
			new string[]
			{
				"SpatialGDKEditor/Private",
			});
	}
}
