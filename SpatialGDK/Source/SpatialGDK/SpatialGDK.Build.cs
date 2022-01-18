// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using System;
using System.ComponentModel;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Diagnostics;
using Tools.DotNETCommon;
using UnrealBuildTool;

public class SpatialGDK : ModuleRules
{
    public SpatialGDK(ReadOnlyTargetRules Target) : base(Target)
    {
        bLegacyPublicIncludePaths = false;
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        // Buildkite can store environment variables like "True" and "False" as "1" and "0", which can't be parsed by bool.TryParse() or bool.Parse()
		string unity_mode_env = Environment.GetEnvironmentVariable("GDK_UNITY_MODE");
		bool unity_mode;
        if (bool.TryParse(unity_mode_env, out unity_mode)) {
            // unity_mode_env is "True", "False" or case-insensitive versions of these
    		bUseUnity = unity_mode;
        } else if (unity_mode_env.Equals("0")) {
            bUseUnity = false;
        } else if (unity_mode_env.Equals("1")) {
            bUseUnity = true;
        } else {
            throw new FormatException("Value of GDK_UNITY_MODE environment variable was not recognized as a valid Boolean.");
        }

        PrivateIncludePaths.Add("SpatialGDK/Private");

        var WorkerSDKPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "Public", "WorkerSDK"));

        PublicIncludePaths.Add(WorkerSDKPath); // Worker SDK uses a different include format <improbable/x.h>
        PrivateIncludePaths.Add(WorkerSDKPath);

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "EngineSettings",
                "Http",
                "InputCore",
                "OnlineSubsystemUtils",
                "Projects",
                "ReplicationGraph",
                "Sockets",
                "Slate",
                "UMG"
            });

        if (Target.bWithPushModel)
        {
            PublicDependencyModuleNames.Add("NetCore");
        }

        if (Target.bBuildDeveloperTools || (Target.Configuration != UnrealTargetConfiguration.Shipping &&
                                            Target.Configuration != UnrealTargetConfiguration.Test))
        {
            PublicDependencyModuleNames.Add("GameplayDebugger");
            PublicDefinitions.Add("WITH_GAMEPLAY_DEBUGGER=1");
        }
        else
        {
            PublicDefinitions.Add("WITH_GAMEPLAY_DEBUGGER=0");
        }

        if (Target.bBuildEditor)
        {
            PublicDependencyModuleNames.Add("UnrealEd");
            PublicDependencyModuleNames.Add("SpatialGDKServices");
        }

        if (Target.bWithPerfCounters)
        {
            PublicDependencyModuleNames.Add("PerfCounters");
        }

        var WorkerLibraryDir = Path.Combine(ModuleDirectory, "..", "..", "Binaries", "ThirdParty", "Improbable", Target.Platform.ToString());

        string LibPrefix = "libimprobable_";
        string ImportLibSuffix = ".so";
        string SharedLibSuffix = ".so";
        bool bAddDelayLoad = false;

        if (Target.Platform == UnrealTargetPlatform.Win32 || Target.Platform == UnrealTargetPlatform.Win64)
        {
            LibPrefix = "improbable_";
            ImportLibSuffix = ".lib";
            SharedLibSuffix = ".dll";
            bAddDelayLoad = true;
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            ImportLibSuffix = SharedLibSuffix = ".dylib";
        }
        else if (Target.Platform == UnrealTargetPlatform.PS4)
        {
            ImportLibSuffix = "_stub.a";
            SharedLibSuffix = ".prx";
            bAddDelayLoad = true;
        }
        else if (Target.Platform == UnrealTargetPlatform.XboxOne)
        {
            LibPrefix = "improbable_";
            ImportLibSuffix = ".lib";
            SharedLibSuffix = ".dll";
            // We don't set bAddDelayLoad = true here, because we get "unresolved external symbol __delayLoadHelper2".
            // See: https://www.fmod.org/questions/question/deploy-issue-on-xboxone-with-unrealengine-4-14/
        }
        else if (Target.Platform == UnrealTargetPlatform.IOS)
        {
            ImportLibSuffix = SharedLibSuffix = "_static.a";
        }
        else if(!(Target.Platform == UnrealTargetPlatform.Linux || Target.Platform == UnrealTargetPlatform.Android))
        {
            throw new System.Exception(System.String.Format("Unsupported platform {0}", Target.Platform.ToString()));
        }

        string WorkerImportLib = System.String.Format("{0}worker{1}", LibPrefix, ImportLibSuffix);
        string WorkerSharedLib = System.String.Format("{0}worker{1}", LibPrefix, SharedLibSuffix);

        if (Target.Platform != UnrealTargetPlatform.Android)
        {
            RuntimeDependencies.Add(Path.Combine(WorkerLibraryDir, WorkerSharedLib), StagedFileType.NonUFS);
            if (bAddDelayLoad)
            {
                PublicDelayLoadDLLs.Add(WorkerSharedLib);
            }

            WorkerImportLib = Path.Combine(WorkerLibraryDir, WorkerImportLib);
            PublicRuntimeLibraryPaths.Add(WorkerLibraryDir);

            PublicAdditionalLibraries.Add(WorkerImportLib);
        }
        else
        {
            var WorkerLibraryPaths = new List<string>
            {
                Path.Combine(WorkerLibraryDir, "arm64-v8a"),
                Path.Combine(WorkerLibraryDir, "armeabi-v7a"),
                Path.Combine(WorkerLibraryDir, "x86_64"),
            };

            string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
            AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "SpatialGDK_APL.xml"));

            PublicRuntimeLibraryPaths.AddRange(WorkerLibraryPaths);

            var WorkerLibraries = new List<string>
            {
                Path.Combine(WorkerLibraryDir, "arm64-v8a", WorkerSharedLib),
                Path.Combine(WorkerLibraryDir, "armeabi-v7a", WorkerSharedLib),
                Path.Combine(WorkerLibraryDir, "x86_64", WorkerSharedLib),
            };

            PublicAdditionalLibraries.AddRange(WorkerLibraries);
        }
    }
}
