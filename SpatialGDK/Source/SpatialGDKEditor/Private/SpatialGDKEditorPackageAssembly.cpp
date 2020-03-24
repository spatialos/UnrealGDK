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

DEFINE_LOG_CATEGORY(LogSpatialGDKEditorPackageAssembly);

#define LOCTEXT_NAMESPACE "SpatialGDKEditorPackageAssembly"



FSpatialGDKPackageAssembly::FSpatialGDKPackageAssembly() : CurrentAssemblyTarget{ EPackageAssemblyTarget::NONE }
{

}

static FString GetStagingDir()
{
	return FPaths::ConvertRelativePathToFull((FPaths::IsProjectFilePathSet() ? FPaths::GetPath(FPaths::GetProjectFilePath()) : FPaths::RootDir() / FApp::GetProjectName()) / TEXT("..") / TEXT("spatial") / TEXT("build") / TEXT("unreal"));
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

static void WriteSimulatedPlayerWorkerShellScript()
{
	FString ShellScript = FString::Printf(TEXT(
		"#!/bin/bash\n"
		"NEW_USER=unrealworker\n"
		"WORKER_ID=$1\n"
		"shift 1\n"
		"\n"
		"# 2>/dev/null silences errors by redirecting stderr to the null device. This is done to prevent errors when a machine attempts to add the same user more than once.\n"
		"useradd $NEW_USER -m -d /improbable/logs/ >> \"/improbable/logs/${WORKER_ID}.log\" 2>&1\n"
		"chown -R $NEW_USER:$NEW_USER $(pwd) >> \"/improbable/logs/${WORKER_ID}.log\" 2>&1\n"
		"chmod -R o+rw /improbable/logs >> \"/improbable/logs/${WORKER_ID}.log\" 2>&1\n"
		"SCRIPT=\"$(pwd)/%s.sh\"\n"
		"chmod +x $SCRIPT >> \"/improbable/logs/${WORKER_ID}.log\" 2>&1\n"
		"\n"
		"echo \"Trying to launch worker %s with id ${WORKER_ID}\" > \"/improbable/logs/${WORKER_ID}.log\"\n"
		"gosu $NEW_USER \"${SCRIPT}\" \"$@\" >> \"/improbable/logs/${WORKER_ID}.log\" 2>&1\n"
		), FApp::GetProjectName());

	FString StagingDir = GetStagingDir();
	FString OutputFile = StagingDir / TEXT("LinuxNoEditor") / TEXT("StartSimulatedClient.sh");
	FFileHelper::SaveStringToFile(ShellScript, *OutputFile, FFileHelper::EEncodingOptions::ForceAnsi);
}

static void WriteSimulatedPlayerCoordinatorShellScript()
{
	FString ShellScript = FString::Printf(TEXT(
		"#!/bin/sh\n"
		"\n"
		"# Some clients are quite large so in order to avoid running out of disk space on the node we attempt to delete the zip\n"
		"WORKER_ZIP_DIR=\"/tmp/runner_source/\"\n"
		"if [ -d \"$WORKER_ZIP_DIR\" ]; then\n"
		"  rm -rf \"$WORKER_ZIP_DIR\"\n"
		"fi\n"
		"\n"
		"sleep 5\n"
		"\n"
		"chmod +x WorkerCoordinator.exe\n"
		"chmod +x StartSimulatedClient.sh\n"
		"chmod +x %s.sh\n"
		"\n"
		"mono WorkerCoordinator.exe $@ 2> /improbable/logs/CoordinatorErrors.log\n"
		), FApp::GetProjectName());

	FString StagingDir = GetStagingDir();
	FString OutputFile = StagingDir / TEXT("LinuxNoEditor") / TEXT("StartCoordinator.sh");
	FFileHelper::SaveStringToFile(ShellScript, *OutputFile, FFileHelper::EEncodingOptions::ForceAnsi);
}

static void ZipWorker(const FString& WorkerName, const FString& ZipName, TFunction<void(FString,double)> Func)
{
	//write shell script and zip folder
	FString ProjectPath = FPaths::IsProjectFilePathSet() ? FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath()) : FPaths::RootDir() / FApp::GetProjectName() / FApp::GetProjectName() + TEXT(".uproject");

	FString SourcePath = FPaths::ConvertRelativePathToFull((FPaths::IsProjectFilePathSet() ? FPaths::GetPath(FPaths::GetProjectFilePath()) : FPaths::RootDir() / FApp::GetProjectName()) / TEXT("..") / TEXT("spatial") / TEXT("build") / TEXT("unreal") / WorkerName);
	FString AssemblyPath = FPaths::ConvertRelativePathToFull((FPaths::IsProjectFilePathSet() ? FPaths::GetPath(FPaths::GetProjectFilePath()) : FPaths::RootDir() / FApp::GetProjectName()) / TEXT("..") / TEXT("spatial") / TEXT("build") / TEXT("assembly") / TEXT("worker") / ZipName);
	FString CommandLine = FString::Printf(TEXT("-ScriptsForProject=\"%s\" ZipUtils -add=\"%s\" -archive=\"%s\""),
		*ProjectPath, *SourcePath, *AssemblyPath);

	AsyncTask(ENamedThreads::GameThread, [CommandLine, Func]() {
		IUATHelperModule::Get().CreateUatTask(CommandLine,
			LOCTEXT("ZipAssemblyDisplayName", "Spatial Cloud"),
			LOCTEXT("ZipAssemblyDescription", "Zip Cloud Deployment Assembly"),
			LOCTEXT("ZipAssemblyShortName", "Cloud Assembly"),
			FEditorStyle::GetBrush(TEXT("MainFrame.CookContent")),
			TFunction<void(FString, double)>(Func)
			);
		});
}

static void Build(const FString& CommandLine, TFunction<void(FString, double)> Func)
{
	AsyncTask(ENamedThreads::GameThread, [CommandLine, Func]() {
		IUATHelperModule::Get().CreateUatTask(CommandLine,
			LOCTEXT("BuildAssemblyDisplayName", "Spatial Cloud"),
			LOCTEXT("BuildAssemblyDescription", "Build Cloud Deployment Assembly"),
			LOCTEXT("BuildAssemblyShortName", "Cloud Assembly"),
			FEditorStyle::GetBrush(TEXT("MainFrame.CookContent")),
			TFunction<void(FString, double)>(Func)
			);
		});
}

static FString GetServerBuildCommand(const FString& OptionalParams)
{
	FString ProjectPath = FPaths::IsProjectFilePathSet() ? FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath()) : FPaths::RootDir() / FApp::GetProjectName() / FApp::GetProjectName() + TEXT(".uproject");
	FString StagingDir = GetStagingDir() / TEXT("LinuxServer");
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
	return CommandLine;
}

static FString GenSimulatedPlayerBuildCommand(const FString& OptionParams)
{
	FString ProjectPath = FPaths::IsProjectFilePathSet() ? FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath()) : FPaths::RootDir() / FApp::GetProjectName() / FApp::GetProjectName() + TEXT(".uproject");
	FString StagingDir = GetStagingDir() / TEXT("LinuxNoEditor");
	FString OptionalParams;
	FString CommandLine = FString::Printf(TEXT("-ScriptsForProject=\"%s\" BuildCookRun -build -project=\"%s\" -nop4 -clientconfig=%s -serverconfig=%s -utf8output -cook -stage -package -unversioned -compressed -stagingdirectory=\"%s\"  -fileopenlog -SkipCookingEditorContent -platform=%s -targetplatform=%s -nullrhi -ue4exe=\"%s\" %s"),
		*ProjectPath,
		*ProjectPath,
		TEXT("Development"),
		TEXT("Development"),
		*StagingDir,
		TEXT("Linux"),
		TEXT("Linux"),
		*FUnrealEdMisc::Get().GetExecutableForCommandlets(),
		*OptionalParams
		);
	return CommandLine;

}

static FString GenClientBuildCommand(const FString &OptionParams)
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
	return CommandLine;
}

void FSpatialGDKPackageAssembly::BuildServerWorker()
{
	auto ZipLambda = [this](FString, double) { CurrentAssemblyTarget = EPackageAssemblyTarget::NONE; };
	auto BuildLambda = [this, ZipLambda](FString Result, double) {
		if (Result == TEXT("Completed"))
		{
			WriteStartWorkerScript();
			ZipWorker(TEXT("LinuxServer"), TEXT("UnrealWorker@Linux.zip"), ZipLambda);
		}
		else
		{
			CurrentAssemblyTarget = EPackageAssemblyTarget::NONE;
		}
	};
	CurrentAssemblyTarget = EPackageAssemblyTarget::BUILD_SERVER;
	Build(GetServerBuildCommand(TEXT("")), BuildLambda);
}

void FSpatialGDKPackageAssembly::BuildClientWorker()
{
	auto ZipLambda = [this](FString, double) { CurrentAssemblyTarget = EPackageAssemblyTarget::NONE; };
	auto BuildLambda = [this, ZipLambda](FString Result, double) {
		if (Result == TEXT("Completed"))
		{
			ZipWorker(TEXT("WindowsNoEditor"), TEXT("UnrealClient@Windows.zip"), ZipLambda);
		}
		else
		{
			CurrentAssemblyTarget = EPackageAssemblyTarget::NONE;
		}
	};
	CurrentAssemblyTarget = EPackageAssemblyTarget::BUILD_CLIENT;
	Build(GenClientBuildCommand(TEXT("")), BuildLambda);
}

static void AddFilesForSimulatedPlayer()
{
	WriteSimulatedPlayerWorkerShellScript();
	WriteSimulatedPlayerCoordinatorShellScript();
	FString WorkerCoordinatorPath = FPaths::ConvertRelativePathToFull(FSpatialGDKServicesModule::GetSpatialGDKPluginDirectory(TEXT("SpatialGDK/Binaries/ThirdParty/Improbable/Programs/WorkerCoordinator")));
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.DirectoryExists(*WorkerCoordinatorPath))
	{
		FString StagingDir = GetStagingDir() / TEXT("LinuxNoEditor");
		PlatformFile.CopyDirectoryTree(*StagingDir, *WorkerCoordinatorPath, true);
	}
}

void FSpatialGDKPackageAssembly::BuildSimulatedPlayerWorker()
{
	auto ZipLambda = [this](FString, double) { CurrentAssemblyTarget = EPackageAssemblyTarget::NONE; };
	auto BuildLambda = [this, ZipLambda](FString Result, double) {
		if (Result == TEXT("Completed"))
		{
			AddFilesForSimulatedPlayer();
			ZipWorker(TEXT("LinuxNoEditor"), TEXT("UnrealSimulatedPlayer@Linux.zip"), ZipLambda);
		}
		else
		{
			CurrentAssemblyTarget = EPackageAssemblyTarget::NONE;
		}
	};
	CurrentAssemblyTarget = EPackageAssemblyTarget::BUILD_CLIENT;
	Build(GenClientBuildCommand(TEXT("")), BuildLambda);
}

void FSpatialGDKPackageAssembly::BuildNext()
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	auto lambda = [this](FString Result, double) {
		if (Result == TEXT("Completed"))
		{
			BuildNext();
		}
		else
		{
			CurrentAssemblyTarget = EPackageAssemblyTarget::NONE;
		}
	};
	switch (CurrentAssemblyTarget)
	{
	case EPackageAssemblyTarget::START_ALL:
		CurrentAssemblyTarget = EPackageAssemblyTarget::BUILD_CLIENT;
		Build(GenClientBuildCommand(TEXT("")), lambda);
		break;
	case EPackageAssemblyTarget::BUILD_CLIENT:
		CurrentAssemblyTarget = EPackageAssemblyTarget::ZIP_CLIENT;
		ZipWorker(TEXT("WindowsNoEditor"), TEXT("UnrealClient@Windows.zip"), lambda);
		break;
	case EPackageAssemblyTarget::ZIP_CLIENT:
		CurrentAssemblyTarget = EPackageAssemblyTarget::BUILD_SERVER;
		Build(GetServerBuildCommand(TEXT("")), lambda);
		break;
	case EPackageAssemblyTarget::BUILD_SERVER:
		CurrentAssemblyTarget = EPackageAssemblyTarget::ZIP_SERVER;
		WriteStartWorkerScript();
		ZipWorker(TEXT("LinuxServer"), TEXT("UnrealWorker@Linux.zip"), lambda);
	case EPackageAssemblyTarget::ZIP_SERVER:
		if (SpatialGDKSettings->IsSimulatedPlayersEnabled())
		{
			CurrentAssemblyTarget = EPackageAssemblyTarget::BUILD_SIMULATED_PLAYERS;
			Build(GenSimulatedPlayerBuildCommand(TEXT("")), lambda);
		}
		else
		{
			CurrentAssemblyTarget = EPackageAssemblyTarget::NONE;
		}
		break;
	case EPackageAssemblyTarget::BUILD_SIMULATED_PLAYERS:
		CurrentAssemblyTarget = EPackageAssemblyTarget::ZIP_SIMULATED_PLAYERS;
		AddFilesForSimulatedPlayer();
		ZipWorker(TEXT("LinuxNoEditor"), TEXT("UnrealSimulatedPlayer@Linux.zip"), lambda);
	default:
		CurrentAssemblyTarget = EPackageAssemblyTarget::NONE;
		;
	}
}

void FSpatialGDKPackageAssembly::BuildAll()
{
	if (CurrentAssemblyTarget == EPackageAssemblyTarget::NONE)
	{
		CurrentAssemblyTarget = EPackageAssemblyTarget::START_ALL;
		BuildNext();
	}
}

bool FSpatialGDKPackageAssembly::CanBuild() const
{
	return CurrentAssemblyTarget == EPackageAssemblyTarget::NONE;
}


#undef LOCTEXT_NAMESPACE
