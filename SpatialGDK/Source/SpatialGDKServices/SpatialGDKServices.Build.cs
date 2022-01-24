// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
using System;
using UnrealBuildTool;

public class SpatialGDKServices : ModuleRules
{
    public SpatialGDKServices(ReadOnlyTargetRules Target) : base(Target)
    {
        bLegacyPublicIncludePaths = false;
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bUseUnity = BuildUtils.GetUnityModeSetting();

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
                "Sockets",
                "HTTP"
            }
        );
    }
}

public static class BuildUtils
{
    public static bool GetUnityModeSetting()
    {
        string unity_mode_env = Environment.GetEnvironmentVariable("GDK_UNITY_MODE");
        if (unity_mode_env != null)
        {
            // Buildkite can store environment variables like "True" and "False" as "1" and "0", which can't be parsed by bool.TryParse() or bool.Parse()
            bool unity_mode;
            if (bool.TryParse(unity_mode_env, out unity_mode))
            {
                // unity_mode_env is "True", "False" or case-insensitive versions of these
                return unity_mode;
            }
            else if (unity_mode_env.Equals("1"))
            {
                return true;
            }
            else if (unity_mode_env.Equals("0"))
            {
                return false;
            }
        }
        // Environment variable not set, default to non-unity mode
        return false;
    }
}
