// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using UnrealBuildTool;

public class SpatialOSEditorToolbar : ModuleRules
{
	public SpatialOSEditorToolbar(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		bFasterWithoutUnity = true;

		PublicIncludePaths.AddRange(
			new string[] {
				"SpatialOSEditorToolbar/Public",
				"SpatialOS/Public"
			});
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"SpatialOSEditorToolbar/Private",
			});
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Json",
				"JsonUtilities"
			});
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"InputCore",
				"UnrealEd",
				"LevelEditor",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
			});
	}
}
