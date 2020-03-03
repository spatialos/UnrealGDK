// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using System.Collections.Generic;
using System.IO;
using UnrealBuildTool;

public class SpatialGDKCommon : ModuleRules
{
	public SpatialGDKCommon(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bFasterWithoutUnity = true;

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core"
            }
        );
	}
}
