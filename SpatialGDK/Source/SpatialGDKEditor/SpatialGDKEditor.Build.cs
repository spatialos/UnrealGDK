// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using UnrealBuildTool;

public class SpatialGDKEditor : ModuleRules
{
	public SpatialGDKEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		bLegacyPublicIncludePaths = false;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bUseUnity = false;

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
#if UE_5_0_OR_LATER
				"DeveloperToolSettings",
#endif
			});

		PrivateIncludePaths.AddRange(
			new string[]
			{
				"SpatialGDKEditor/Private",
			});
	}
}
