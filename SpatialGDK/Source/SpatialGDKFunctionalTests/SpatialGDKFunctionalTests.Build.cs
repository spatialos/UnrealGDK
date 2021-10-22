// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using UnrealBuildTool;

public class SpatialGDKFunctionalTests : ModuleRules
{
    public SpatialGDKFunctionalTests(ReadOnlyTargetRules Target) : base(Target)
    {
        bLegacyPublicIncludePaths = false;
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bUseUnity = false;

        PrivateIncludePaths.Add(ModuleDirectory);
        
        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "SpatialGDK",
                "Core",
                "CoreUObject",
                "Engine",
                "FunctionalTesting",
                "HTTP",
                "EngineSettings",
                "InputCore",
                "Slate"
            });


        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
            PrivateDependencyModuleNames.Add("SpatialGDKEditor");
            PrivateDependencyModuleNames.Add("SpatialGDKServices");
        }

    }
}
