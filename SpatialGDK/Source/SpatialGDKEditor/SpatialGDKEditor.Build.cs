// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
using System;
using UnrealBuildTool;

public class SpatialGDKEditor : ModuleRules
{
    public SpatialGDKEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        bLegacyPublicIncludePaths = false;
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bUseUnity = BuildUtils.GetUnityModeSetting();

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
