// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using UnrealBuildTool;

public class SpatialGDKFunctionalTests : ModuleRules
{
    public SpatialGDKFunctionalTests(ReadOnlyTargetRules Target) : base(Target)
    {
        bLegacyPublicIncludePaths = false;
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
		string unity_mode_env = System.Environment.GetEnvironmentVariable("UNITY_MODE");
		bool unity_mode = bool.Parse(unity_mode_env);
		bUseUnity = unity_mode;

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
