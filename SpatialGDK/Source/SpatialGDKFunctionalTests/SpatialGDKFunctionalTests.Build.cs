// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
using System;
using UnrealBuildTool;

public class SpatialGDKFunctionalTests : ModuleRules
{
    public SpatialGDKFunctionalTests(ReadOnlyTargetRules Target) : base(Target)
    {
        bLegacyPublicIncludePaths = false;
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
		// Buildkite can store environment variables like "True" and "False" as "1" and "0", which can't be parsed by bool.TryParse() or bool.Parse()
		string unity_mode_env = Environment.GetEnvironmentVariable("GDK_UNITY_MODE");
		bool unity_mode;
        if (bool.TryParse(unity_mode_env, out unity_mode)) {
            // unity_mode_env is "True", "False" or case-insensitive versions of these
    		bUseUnity = unity_mode;
        } else if (unity_mode_env.Equals("0")) {
            bUseUnity = false;
        } else if (unity_mode_env.Equals("1")) {
            bUseUnity = true;
        } else {
            throw new FormatException("Value of GDK_UNITY_MODE environment variable was not recognized as a valid Boolean.");
        }

        PrivateIncludePaths.Add(ModuleDirectory);
        
        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "SpatialGDK",
                "SpatialGDKEditor",
                "SpatialGDKServices",
                "Core",
                "CoreUObject",
                "Engine",
                "FunctionalTesting",
                "HTTP",
                "UnrealEd",
                "EngineSettings",
                "InputCore",
                "Slate"
            });
    }
}
