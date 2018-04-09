// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using UnrealBuildTool;

public class SpatialGDKEditorToolbar : ModuleRules
{
	public SpatialGDKEditorToolbar(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		bFasterWithoutUnity = true;

		PublicIncludePaths.AddRange(
			new string[] {
				"SpatialGDKEditorToolbar/Public",
				"SpatialOS/Public"
			});
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"SpatialGDKEditorToolbar/Private",
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
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"LevelEditor",
				"Projects",
				"Slate",
				"SlateCore",
				"SpatialOS",
				"SpatialGDK",
				"UnrealEd"
			});
	}
}
