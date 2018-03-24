// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using UnrealBuildTool;

public class NUFEditorToolbar : ModuleRules
{
	public NUFEditorToolbar(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		bFasterWithoutUnity = true;

		PublicIncludePaths.AddRange(
			new string[] {
				"NUFEditorToolbar/Public",
				"SpatialOS/Public"
			});
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"NUFEditorToolbar/Private",
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
				"SpatialOS",
				"NUF"
			});
	}
}
