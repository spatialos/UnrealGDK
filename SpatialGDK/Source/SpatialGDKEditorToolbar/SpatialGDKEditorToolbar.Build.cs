// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using UnrealBuildTool;

public class SpatialGDKEditorToolbar : ModuleRules
{
    public SpatialGDKEditorToolbar(ReadOnlyTargetRules Target) : base(Target)
    {
        bLegacyPublicIncludePaths = false;
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
		string unity_mode_env = System.Environment.GetEnvironmentVariable("GDK_UNITY_MODE");
		bool unity_mode = bool.Parse(unity_mode_env);
		bUseUnity = unity_mode;

        PrivateIncludePaths.Add("SpatialGDKEditorToolbar/Private");

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "DesktopPlatform",
                "DesktopWidgets",
                "Engine",
                "EngineSettings",
                "InputCore",
                "IOSRuntimeSettings",
                "LevelEditor",
                "Projects",
                "Slate",
                "SlateCore",
                "EditorStyle",
                "MessageLog",
                "SpatialGDK",
                "SpatialGDKEditor",
                "SpatialGDKFunctionalTests",
                "SpatialGDKServices",
                "UnrealEd",
                "UATHelper"
            }
        );
    }
}
