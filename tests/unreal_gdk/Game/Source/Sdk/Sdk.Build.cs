// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
using System.IO;
using UnrealBuildTool;

public class Sdk : ModuleRules
{
	public Sdk(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        bFasterWithoutUnity = true;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core", "CoreUObject", "Engine", "InputCore", "SpatialGDK"
        });

        PublicDefinitions.Add("SPATIALOS_WORKER_SDK_MOCK_ENABLED");
    }
}
