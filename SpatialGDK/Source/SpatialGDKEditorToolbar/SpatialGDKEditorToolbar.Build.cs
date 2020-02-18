// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using UnrealBuildTool;

public class SpatialGDKEditorToolbar : ModuleRules
{
    public SpatialGDKEditorToolbar(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
#pragma warning disable 0618
        bFasterWithoutUnity = true; // Deprecated in 4.24, replace with bUseUnity = true; once we drop support for 4.23
#pragma warning restore 0618

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
                "SpatialGDKServices",
                "UnrealEd"
            }
        );
    }
}
