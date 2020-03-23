// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorPackageAssembly.h"

#include "CoreMinimal.h"

#include "Async/Async.h"
#include "Misc/App.h"
#include "Misc/FileHelper.h"
#include "UnrealEdMisc.h"
#include "EditorStyle.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKServicesModule.h"

#include "UATHelper/Public/IUATHelperModule.h"

#define LOCTEXT_NAMESPACE "SpatialGDKEditorPackageAssembly"

DEFINE_LOG_CATEGORY(LogSpatialGDKEditorPackageAssembly);

static FString GetStagingDir()
{
	return FPaths::ConvertRelativePathToFull((FPaths::IsProjectFilePathSet() ? FPaths::GetPath(FPaths::GetProjectFilePath()) : FPaths::RootDir() / FApp::GetProjectName()) / TEXT("..") / TEXT("spatial") / TEXT("build") / TEXT("unreal"));
}

static void HandleZipResult(FString, double)
{

}

static void WriteStartWorkerScript()
{
	FString StagingDir = GetStagingDir();
	FString OutputFile = StagingDir / TEXT("LinuxServer") / TEXT("StartWorker.sh");
	FString ShellScript = FString::Printf(TEXT(
		"#!/bin/bash\n"
		"NEW_USER=unrealworker\n"
		"WORKER_ID=$1\n"
		"LOG_FILE=$2\n"
		"shift 2\n"
		"\n"
		"# 2>/dev/null silences errors by redirecting stderr to the null device.This is done to prevent errors when a machine attempts to add the same user more than once.\n"
		"mkdir -p /improbable/logs/UnrealWorker/\n"
		"useradd $NEW_USER -m -d /improbable/logs/UnrealWorker 2>/dev/null\n"
		"chown -R $NEW_USER:$NEW_USER $(pwd) 2>/dev/null\n"
		"chmod -R o+rw /improbable/logs 2>/dev/null\n"
		"\n"
		"# Create log file in case it doesn't exist and redirect stdout and stderr to the file.\n"
		"touch \"${LOG_FILE}\"\n"
		"exec 1>>\"${LOG_FILE}\"\n"
		"exec 2>&1\n"
		"\n"
		"SCRIPT=\"$(pwd)/%sServer.sh\"\n"
		"\n"
		"if [ ! -f $SCRIPT ]; then\n"
		"	echo \"Expected to run ${SCRIPT} but file not found!\"\n"
		"	exit 1\n"
		"fi\n"
		"\n"
		"chmod +x $SCRIPT\n"
		"echo \"Running ${SCRIPT} to start worker...\"\n"
		"gosu $NEW_USER \"${SCRIPT}\" \"$@\"\n"), FApp::GetProjectName());
	FFileHelper::SaveStringToFile(ShellScript, *OutputFile, FFileHelper::EEncodingOptions::ForceAnsi);
}

static void ZipWorker(const FString& WorkerName, const FString& ZipName)
{
	//write shell script and zip folder
	FString ProjectPath = FPaths::IsProjectFilePathSet() ? FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath()) : FPaths::RootDir() / FApp::GetProjectName() / FApp::GetProjectName() + TEXT(".uproject");

	FString SourcePath = FPaths::ConvertRelativePathToFull((FPaths::IsProjectFilePathSet() ? FPaths::GetPath(FPaths::GetProjectFilePath()) : FPaths::RootDir() / FApp::GetProjectName()) / TEXT("..") / TEXT("spatial") / TEXT("build") / TEXT("unreal") / WorkerName);
	FString AssemblyPath = FPaths::ConvertRelativePathToFull((FPaths::IsProjectFilePathSet() ? FPaths::GetPath(FPaths::GetProjectFilePath()) : FPaths::RootDir() / FApp::GetProjectName()) / TEXT("..") / TEXT("spatial") / TEXT("build") / TEXT("assembly") / TEXT("worker") / ZipName);
	FString CommandLine = FString::Printf(TEXT("-ScriptsForProject=\"%s\" ZipUtils -add=\"%s\" -archive=\"%s\""),
		*ProjectPath, *SourcePath, *AssemblyPath);

	AsyncTask(ENamedThreads::GameThread, [CommandLine]() {
		IUATHelperModule::Get().CreateUatTask(CommandLine,
			LOCTEXT("ZipAssemblyDisplayName", "Spatial Cloud"),
			LOCTEXT("ZipAssemblyDescription", "Zip Cloud Deployment Assembly"),
			LOCTEXT("ZipAssemblyShortName", "Cloud Assembly"),
			FEditorStyle::GetBrush(TEXT("MainFrame.CookContent")),
			TFunction<void(FString, double)>(HandleZipResult)
			);
		});
}

static void HandleServerWorkerResult(FString result, double)
{
	if (result == TEXT("Completed"))
	{
		WriteStartWorkerScript();
		ZipWorker(TEXT("LinuxServer"), TEXT("UnrealWorker@Linux.zip"));
	}
}

void BuildServerWorker()
{
	FString ProjectPath = FPaths::IsProjectFilePathSet() ? FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath()) : FPaths::RootDir() / FApp::GetProjectName() / FApp::GetProjectName() + TEXT(".uproject");
	FString StagingDir = GetStagingDir() / TEXT("LinuxServer");
	FString OptionalParams;
	FString CommandLine = FString::Printf(TEXT("-ScriptsForProject=\"%s\" BuildCookRun -build -project=\"%s\" -nop4 -clientconfig=%s -serverconfig=%s -utf8output -cook -stage -package -unversioned -compressed -stagingdirectory=\"%s\"  -fileopenlog -SkipCookingEditorContent -server -serverplatform=%s -noclient -ue4exe=\"%s\" %s"),
		*ProjectPath,
		*ProjectPath,
		TEXT("Development"),
		TEXT("Development"),
		*StagingDir,
		TEXT("Linux"),
		*FUnrealEdMisc::Get().GetExecutableForCommandlets(),
		*OptionalParams
		);

	IUATHelperModule::Get().CreateUatTask(CommandLine,
		LOCTEXT("BuildAssemblyDisplayName", "Spatial Cloud"),
		LOCTEXT("BuildAssemblyDescription", "Build Cloud Deployment Assembly"),
		LOCTEXT("BuildAssemblyShortName", "Cloud Assembly"),
		FEditorStyle::GetBrush(TEXT("MainFrame.CookContent")),
		TFunction<void(FString, double)>(HandleServerWorkerResult)
		);
}

static void HandleClientWorkerResult(FString result, double)
{
	if (result == TEXT("Completed"))
	{
		ZipWorker(TEXT("WindowsNoEditor"), TEXT("UnrealClient@Windows.zip"));
	}
}

void BuildClientWorker()
{
	FString ProjectPath = FPaths::IsProjectFilePathSet() ? FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath()) : FPaths::RootDir() / FApp::GetProjectName() / FApp::GetProjectName() + TEXT(".uproject");
	FString StagingDir = GetStagingDir() / TEXT("WindowsNoEditor");
	FString OptionalParams;
	FString CommandLine = FString::Printf(TEXT("-ScriptsForProject=\"%s\" BuildCookRun -build -project=\"%s\" -nop4 -clientconfig=%s -serverconfig=%s -utf8output -cook -stage -package -unversioned -compressed -stagingdirectory=\"%s\"  -fileopenlog -SkipCookingEditorContent -platform=%s -targetplatform=%s -ue4exe=\"%s\" %s"),
		*ProjectPath,
		*ProjectPath,
		TEXT("Development"),
		TEXT("Development"),
		*StagingDir,
		TEXT("Win64"),
		TEXT("Win64"),
		*FUnrealEdMisc::Get().GetExecutableForCommandlets(),
		*OptionalParams
		);

	IUATHelperModule::Get().CreateUatTask(CommandLine,
		LOCTEXT("BuildAssemblyDisplayName", "Spatial Cloud"),
		LOCTEXT("BuildAssemblyDescription", "Build Cloud Deployment Assembly"),
		LOCTEXT("BuildAssemblyShortName", "Cloud Assembly"),
		FEditorStyle::GetBrush(TEXT("MainFrame.CookContent")),
		TFunction<void(FString, double)>(HandleClientWorkerResult)
		);
}

void BuildSimulatedPlayerWorker()
{

}

bool SpatialGDKBuildAssemblyServerWorker()
{
	BuildServerWorker();
	return true;
}

bool SpatialGDKBuildAssemblyClientWorker()
{
	BuildClientWorker();
	return true;
}

bool SpatialGDKBuildAssemblySimulatedPlayerWorker()
{
	return false;
}

#undef LOCTEXT_NAMESPACE
