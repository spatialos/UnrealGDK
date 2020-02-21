// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using UnrealBuildTool;

public class SpatialView : ModuleRules
{
	public SpatialView(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bFasterWithoutUnity = true;

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core"
			});
	}
}
