// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using UnrealBuildTool;

public class NUF : ModuleRules
{
	public NUF(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Sockets", "OnlineSubsystemUtils", "SpatialOS" });

		if (UEBuildConfiguration.bBuildEditor == true)
		{
			// Required by UNUFGameInstance::StartPlayInEditorGameInstance.
			PublicDependencyModuleNames.Add("UnrealEd");
		}
	}
}
