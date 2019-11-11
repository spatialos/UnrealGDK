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
    public SpatialGDK(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        bFasterWithoutUnity = true;

        PrivateIncludePaths.Add("SpatialGDK/Private");

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "EngineSettings",
                "Projects",
                "OnlineSubsystemUtils",
                "InputCore",
                "Sockets",
            });

		if (Target.bBuildEditor)
		{
			PublicDependencyModuleNames.Add("UnrealEd");
			PublicDependencyModuleNames.Add("SpatialGDKServices");
		}

        if (Target.bWithPerfCounters)
        {
            PublicDependencyModuleNames.Add("PerfCounters");
        }

        var WorkerLibraryDir = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "Binaries", "ThirdParty", "Improbable", Target.Platform.ToString()));

        string LibPrefix = "improbable_";
        string ImportLibSuffix = "";
        string SharedLibSuffix = "";
        bool bAddDelayLoad = false;

        if (Target.Platform == UnrealTargetPlatform.Win32 || Target.Platform == UnrealTargetPlatform.Win64)
        {
            ImportLibSuffix = ".lib";
            SharedLibSuffix = ".dll";
            bAddDelayLoad = true;
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            LibPrefix = "libimprobable_";
            ImportLibSuffix = SharedLibSuffix = ".dylib";
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            LibPrefix = "libimprobable_";
            ImportLibSuffix = SharedLibSuffix = ".so";
        }
        else if (Target.Platform == UnrealTargetPlatform.PS4)
        {
            LibPrefix = "libimprobable_";
            ImportLibSuffix = "_stub.a";
            SharedLibSuffix = ".prx";
            bAddDelayLoad = true;
        }
        else if (Target.Platform == UnrealTargetPlatform.XboxOne)
        {
            ImportLibSuffix = ".lib";
            SharedLibSuffix = ".dll";
            // We don't set bAddDelayLoad = true here, because we get "unresolved external symbol __delayLoadHelper2".
            // See: https://www.fmod.org/questions/question/deploy-issue-on-xboxone-with-unrealengine-4-14/
        }
        else if (Target.Platform == UnrealTargetPlatform.IOS)
        {
            LibPrefix = "libimprobable_";
            ImportLibSuffix = SharedLibSuffix = "_static.a";
        }
        else
        {
            throw new System.Exception(System.String.Format("Unsupported platform {0}", Target.Platform.ToString()));
        }

        string WorkerImportLib = System.String.Format("{0}worker{1}", LibPrefix, ImportLibSuffix);
        string WorkerSharedLib = System.String.Format("{0}worker{1}", LibPrefix, SharedLibSuffix);

        PublicAdditionalLibraries.AddRange(new[] { Path.Combine(WorkerLibraryDir, WorkerImportLib) });
        PublicLibraryPaths.Add(WorkerLibraryDir);
        RuntimeDependencies.Add(Path.Combine(WorkerLibraryDir, WorkerSharedLib), StagedFileType.NonUFS);
        if (bAddDelayLoad)
        {
            PublicDelayLoadDLLs.Add(WorkerSharedLib);
        }
	}
}
