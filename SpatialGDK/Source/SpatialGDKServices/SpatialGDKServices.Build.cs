// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using UnrealBuildTool;

public class SpatialGDKServices : ModuleRules
{
	public SpatialGDKServices(ReadOnlyTargetRules Target) : base(Target)
	{
		bLegacyPublicIncludePaths = false;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		string unity_mode_env = System.Environment.GetEnvironmentVariable("UNITY_MODE");
		bool unity_mode = bool.Parse(unity_mode_env);
		bUseUnity = unity_mode;

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"EditorStyle",
				"Engine",
				"OutputLog",
				"Slate",
				"SlateCore",
				"Core",
				"CoreUObject",
				"EngineSettings",
				"Json",
				"JsonUtilities",
				"UnrealEd",
				"Sockets",
				"HTTP"
			}
		);
	}
}
