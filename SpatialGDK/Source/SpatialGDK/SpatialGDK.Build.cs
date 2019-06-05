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
			PublicDependencyModuleNames.Add("SpatialGDKEditorToolbar");
		}

        if (Target.bWithPerfCounters)
        {
            PublicDependencyModuleNames.Add("PerfCounters");
        }

        var WorkerLibraryDir = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "Binaries", "ThirdParty", "Improbable", Target.Platform.ToString()));

        string LibPrefix = "";
        string ImportLibSuffix = "";
        string SharedLibSuffix = "";
        bool bAddDelayLoad = false;

        switch (Target.Platform)
        {
            case UnrealTargetPlatform.Win32:
            case UnrealTargetPlatform.Win64:
                ImportLibSuffix = ".lib";
                SharedLibSuffix = ".dll";
                bAddDelayLoad = true;
                break;
            case UnrealTargetPlatform.Mac:
                LibPrefix = "lib";
                ImportLibSuffix = SharedLibSuffix = ".dylib";
                break;
            case UnrealTargetPlatform.Linux:
                LibPrefix = "lib";
                ImportLibSuffix = SharedLibSuffix = ".so";
                break;
            case UnrealTargetPlatform.PS4:
                LibPrefix = "lib";
                ImportLibSuffix = "_stub.a";
                SharedLibSuffix = ".prx";
                bAddDelayLoad = true;
                break;
            case UnrealTargetPlatform.XboxOne:
                ImportLibSuffix = ".lib";
                SharedLibSuffix = ".dll";
                // We don't set bAddDelayLoad = true here, because we get "unresolved external symbol __delayLoadHelper2".
                // See: https://www.fmod.org/questions/question/deploy-issue-on-xboxone-with-unrealengine-4-14/
                break;
            case UnrealTargetPlatform.IOS:
                LibPrefix = "lib";
                ImportLibSuffix = SharedLibSuffix = "_static_fullylinked.a";
                break;
            default:
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
