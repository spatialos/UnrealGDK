// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using System;
using System.ComponentModel;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Diagnostics;
using UnrealBuildTool;

public class SpatialGDK : ModuleRules
{
#if !WITH_FORWARDED_MODULE_RULES_CTOR
    // Backwards compatibility with Unreal 4.15
    public SpatialGDK(TargetInfo Target)
#else
    // Unreal 4.16+
    public SpatialGDK(ReadOnlyTargetRules Target) : base(Target)
#endif
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        bFasterWithoutUnity = true;

        PublicIncludePaths.AddRange(
            new string[] 
            {
                "SpatialGDK/Public",
                "SpatialGDK/Public/WorkerSdk",
                "SpatialGDK/Generated/User",
                "SpatialGDK/Generated/Std",
                "SpatialGDK/Generated/UClasses",
                "SpatialGDK/Legacy",
                "SpatialGDK/Legacy/Deprecated"
            });
        
        PrivateIncludePaths.AddRange( 
            new string[] 
            {
                "SpatialGDK/Private"
            });

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "OnlineSubsystemUtils",
                "PhysXVehicles",
                "InputCore"
            });

		if (UEBuildConfiguration.bBuildEditor == true)
		{
			// Required by USpatialGameInstance::StartPlayInEditorGameInstance.
			PublicDependencyModuleNames.Add("UnrealEd");
		}

   		var CoreSdkLibraryDir = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "Binaries", "ThirdParty", "Improbable", Target.Platform.ToString()));
        string CoreSdkShared;

        if (Target.Platform == UnrealTargetPlatform.Win32 || Target.Platform == UnrealTargetPlatform.Win64)
        {
            CoreSdkShared = Path.Combine(CoreSdkLibraryDir, "CoreSdkDll.dll");
            PublicAdditionalLibraries.AddRange(new[] { Path.Combine(CoreSdkLibraryDir, "CoreSdkDll.lib") });
        }
        else
        {
            CoreSdkShared = Path.Combine(CoreSdkLibraryDir, "libCoreSdkDll.so");
            PublicAdditionalLibraries.AddRange(new[] { CoreSdkShared });
        }

        RuntimeDependencies.Add(CoreSdkShared, StagedFileType.NonUFS);

        PublicLibraryPaths.Add(CoreSdkLibraryDir);
        PublicDelayLoadDLLs.Add("CoreSdkDll.dll");

        // Point generated code to the correct API spec.
        Definitions.Add("IMPROBABLE_DLL_API=SPATIALGDK_API");
	}
}