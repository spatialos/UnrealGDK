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
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
#pragma warning disable 0618
		bFasterWithoutUnity = true;				// Deprecated in 4.24, replace with bUseUnity = false; once we drop support for 4.23
		if (Target.Version.MinorVersion == 24)	// Due to a bug in 4.24, bFasterWithoutUnity is inversed, fixed in master, so should hopefully roll into the next release, remove this once it does
		{
			bFasterWithoutUnity = false;
		}
#pragma warning restore 0618

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

		var WorkerLibraryPaths = new List<string>
			{
				WorkerLibraryDir,
			};

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
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			LibPrefix = "improbable_";
			WorkerLibraryPaths.AddRange(new string[]
			{
				Path.Combine(WorkerLibraryDir, "arm64-v8a"),
				Path.Combine(WorkerLibraryDir, "armeabi-v7a"),
				Path.Combine(WorkerLibraryDir, "x86_64"),
			});

			string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
			AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "SpatialGDK_APL.xml"));
		}
		else
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
		}

		PublicAdditionalLibraries.Add(WorkerImportLib);
#pragma warning disable 0618
		PublicLibraryPaths.AddRange(WorkerLibraryPaths); // Deprecated in 4.24, replace with PublicRuntimeLibraryPaths or move the full path into PublicAdditionalLibraries once we drop support for 4.23
#pragma warning restore 0618

		// Detect existence of trace library, if present add preprocessor
		string TraceStaticLibPath = "";
		string TraceDynamicLib = "";
		string TraceDynamicLibPath = "";
		if (Target.Platform == UnrealTargetPlatform.Win32 || Target.Platform == UnrealTargetPlatform.Win64)
		{
			TraceStaticLibPath = Path.Combine(WorkerLibraryDir, "trace_dynamic.lib");
			TraceDynamicLib = "trace_dynamic.dll";
			TraceDynamicLibPath = Path.Combine(WorkerLibraryDir, TraceDynamicLib);
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			TraceStaticLibPath = Path.Combine(WorkerLibraryDir, "libtrace_dynamic.so");
			TraceDynamicLib = "libtrace_dynamic.so";
			TraceDynamicLibPath = Path.Combine(WorkerLibraryDir, TraceDynamicLib);
		}

		if (File.Exists(TraceStaticLibPath) && File.Exists(TraceDynamicLibPath))
		{
			Log.TraceInformation("Detection of trace libraries found at {0} and {1}, enabling trace functionality.", TraceStaticLibPath, TraceDynamicLibPath);
			PublicDefinitions.Add("TRACE_LIB_ACTIVE=1");

			PublicAdditionalLibraries.Add(TraceStaticLibPath);

			RuntimeDependencies.Add(TraceDynamicLibPath, StagedFileType.NonUFS);
			if (bAddDelayLoad)
			{
				PublicDelayLoadDLLs.Add(TraceDynamicLib);
			}
		}
		else
		{
			Log.TraceInformation("Didn't find trace libraries at {0} and {1}, disabling trace functionality.", TraceStaticLibPath, TraceDynamicLibPath);
			PublicDefinitions.Add("TRACE_LIB_ACTIVE=0");
		}
	}
}
