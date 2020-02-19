// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using System;
using System.ComponentModel;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Diagnostics;
using Tools.DotNETCommon;
using UnrealBuildTool;

public class SpatialGDKWorkerWrapper : ModuleRules
{
	public SpatialGDKWorkerWrapper(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bFasterWithoutUnity = true;

		PrivateIncludePaths.Add("SpatialGDKWorkerWrapper/UnrealWorkerWrapperPrototype/workers/cpp/gdk/include");
		PrivateIncludePaths.Add("SpatialGDK/Public/WorkerSDK");

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"EngineSettings",
				"UnrealEd",
				"Json",
				"JsonUtilities"
			});

		// Load the worker library
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

	}
}
