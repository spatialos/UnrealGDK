// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using System;
using System.ComponentModel;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Diagnostics;
#if UE_5_0_OR_LATER
using EpicGames.Core;
#else
using Tools.DotNETCommon;
#endif
using UnrealBuildTool;

public class SpatialGDK : ModuleRules
{
    public SpatialGDK(ReadOnlyTargetRules Target) : base(Target)
    {
        bLegacyPublicIncludePaths = false;
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        bUseUnity = false;

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
#if UE_5_0_OR_LATER
                "HTTP",
#else
                "Http",
#endif
                "InputCore",
                "OnlineSubsystemUtils",
                "Projects",
                "ReplicationGraph",
                "Sockets",
                "Slate",
                "UMG"
            });

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "NetCore",
            });

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

#if UE_5_0_OR_LATER
        if (Target.Platform == UnrealTargetPlatform.Win64)
#else
        if (Target.Platform == UnrealTargetPlatform.Win32 || Target.Platform == UnrealTargetPlatform.Win64)
#endif
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
#if !UE_5_0_OR_LATER
        else if (Target.Platform == UnrealTargetPlatform.XboxOne)
        {
            LibPrefix = "improbable_";
            ImportLibSuffix = ".lib";
            SharedLibSuffix = ".dll";
            // We don't set bAddDelayLoad = true here, because we get "unresolved external symbol __delayLoadHelper2".
            // See: https://www.fmod.org/questions/question/deploy-issue-on-xboxone-with-unrealengine-4-14/
        }
#endif
        else if (Target.Platform == UnrealTargetPlatform.IOS)
        {
            ImportLibSuffix = SharedLibSuffix = "_static.a";
        }
        else if(!(Target.Platform == UnrealTargetPlatform.Linux || Target.Platform == UnrealTargetPlatform.Android))
        {
            throw new System.Exception(System.String.Format("Unsupported platform {0}", Target.Platform.ToString()));
        }

#if UE_5_0_OR_LATER
        if (Target.Platform.IsInGroup(UnrealPlatformGroup.XboxCommon))
        {
            bAddDelayLoad = false;
            // We don't want bAddDelayLoad here, because we get "unresolved external symbol __delayLoadHelper2".
            // See: https://www.fmod.org/questions/question/deploy-issue-on-xboxone-with-unrealengine-4-14/
        }
#endif

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
