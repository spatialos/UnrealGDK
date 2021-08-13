// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using System.Collections.Generic;
using System.IO;

namespace UnrealBuildTool.Rules
{
	public class ScavengersHubGameFramework : ModuleRules
	{
		public ScavengersHubGameFramework(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
			
			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"DeveloperSettings",
					"Http",
					"Json",
					"JsonUtilities",
					"CoreUObject",
                    "Engine",
					"InputCore",
					"SlateCore",
                    "SpatialGDK"
				}
				);
		}
	}
}
