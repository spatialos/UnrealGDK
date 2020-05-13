// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorCommandLineArgsManager.h"

#include "Input/Reply.h"
#include "IOSRuntimeSettings.h"
#include "Logging/LogMacros.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Serialization/JsonSerializer.h"
#include "SpatialCommandUtils.h"
#include "SpatialGDKSettings.h"
#include "SpatialGDKEditorSettings.h"

#ifdef ENABLE_LAUNCHER_DELEGATE
//need this macro before "ILauncherServicesModule.h" to prevent compile error
#define LAUNCHERSERVICES_API

#include "Developer\LauncherServices\Public\ILauncherServicesModule.h"
#include "Developer\LauncherServices\Public\ILauncherWorker.h"
#include "Developer\LauncherServices\Public\ILauncher.h"
#endif

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKEditorCommandLineArgsManager, Log, All);
DEFINE_LOG_CATEGORY(LogSpatialGDKEditorCommandLineArgsManager);

FSpatialGDKEditorCommandLineArgsManager::FSpatialGDKEditorCommandLineArgsManager()
#ifdef ENABLE_LAUNCHER_DELEGATE
	: bAndroidDevice(false)
	, bIOSDevice(false)
#endif
{

}

void FSpatialGDKEditorCommandLineArgsManager::Startup()
{
#ifdef ENABLE_LAUNCHER_DELEGATE
	ILauncherServicesModule& LauncherServicesModule = FModuleManager::LoadModuleChecked<ILauncherServicesModule>(TEXT("LauncherServices"));
	LauncherServicesModule.OnCreateLauncherDelegate.AddRaw(this, &FSpatialGDKEditorCommandLineArgsManager::OnCreateLauncher);
#endif
}

#ifdef ENABLE_LAUNCHER_DELEGATE
void FSpatialGDKEditorCommandLineArgsManager::OnLauncherCanceled(double ExecutionTime)
{
	RemoveFromDevice();
}

void FSpatialGDKEditorCommandLineArgsManager::OnLauncherFinished(bool Outcome, double ExecutionTime, int32 ReturnCode)
{
	RemoveFromDevice();
}

void FSpatialGDKEditorCommandLineArgsManager::RemoveFromDevice()
{
	if (bAndroidDevice)
	{
		RemoveFromAndroidDevice();
	}
}

void FSpatialGDKEditorCommandLineArgsManager::OnLaunch(ILauncherWorkerPtr LauncherWorkerPtr, ILauncherProfileRef LauncherProfileRef)
{
	LauncherWorkerPtr->OnCanceled().AddRaw(this, &FSpatialGDKEditorCommandLineArgsManager::OnLauncherCanceled);
	LauncherWorkerPtr->OnCompleted().AddRaw(this, &FSpatialGDKEditorCommandLineArgsManager::OnLauncherFinished);

	bIOSDevice = false;
	bAndroidDevice = false;
	TArray<ILauncherTaskPtr> TaskList;
	LauncherWorkerPtr->GetTasks(TaskList);
	for (int32 idx = 0; idx < TaskList.Num(); ++idx)
	{
		if (TaskList[idx]->GetDesc().Contains(TEXT("android")))
		{
			bAndroidDevice = true;
		}
	}
	if (bAndroidDevice)
	{
		UE_LOG(LogSpatialGDKEditorCommandLineArgsManager, Log, TEXT("Android device launched"));
	}
}

void FSpatialGDKEditorCommandLineArgsManager::OnCreateLauncher(ILauncherRef LauncherRef)
{
	LauncherRef->FLauncherWorkerStartedDelegate.AddRaw(this, &FSpatialGDKEditorCommandLineArgsManager::OnLaunch);
}
#endif

namespace
{
	static FString GetAdbExePath()
	{
		FString AndroidHome = FPlatformMisc::GetEnvironmentVariable(TEXT("ANDROID_HOME"));
		if (AndroidHome.IsEmpty())
		{
			UE_LOG(LogSpatialGDKEditorCommandLineArgsManager, Error, TEXT("Environment variable ANDROID_HOME is not set. Please make sure to configure this."));
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Environment variable ANDROID_HOME is not set. Please make sure to configure this.")));
			return TEXT("");
		}

#if PLATFORM_WINDOWS
		const FString AdbExe = FPaths::ConvertRelativePathToFull(FPaths::Combine(AndroidHome, TEXT("platform-tools/adb.exe")));
#else
		const FString AdbExe = FPaths::ConvertRelativePathToFull(FPaths::Combine(AndroidHome, TEXT("platform-tools/adb")));
#endif

		return AdbExe;
	}
}

FReply FSpatialGDKEditorCommandLineArgsManager::PushToIOSDevice()
{
	const UIOSRuntimeSettings* IOSRuntimeSettings = GetDefault<UIOSRuntimeSettings>();
	FString OutCommandLineArgsFile;

	if (!TryConstructMobileCommandLineArgumentsFile(OutCommandLineArgsFile))
	{
		return FReply::Unhandled();
	}

	FString Executable = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::EngineDir(), TEXT("Binaries/DotNET/IOS/deploymentserver.exe")));
	FString DeploymentServerArguments = FString::Printf(TEXT("copyfile -bundle \"%s\" -file \"%s\" -file \"/Documents/ue4commandline.txt\""), *(IOSRuntimeSettings->BundleIdentifier.Replace(TEXT("[PROJECT_NAME]"), FApp::GetProjectName())), *OutCommandLineArgsFile);

#if PLATFORM_MAC
	DeploymentServerArguments = FString::Printf(TEXT("%s %s"), *Executable, *DeploymentServerArguments);
	Executable = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::EngineDir(), TEXT("Binaries/ThirdParty/Mono/Mac/bin/mono")));
#endif

	TryPushCommandLineArgsToDevice(Executable, DeploymentServerArguments, OutCommandLineArgsFile);
	return FReply::Handled();
}

FReply FSpatialGDKEditorCommandLineArgsManager::PushToAndroidDevice()
{
	const FString AdbExe = GetAdbExePath();
	if (AdbExe.IsEmpty())
	{
		return FReply::Unhandled();
	}

	FString OutCommandLineArgsFile;

	if (!TryConstructMobileCommandLineArgumentsFile(OutCommandLineArgsFile))
	{
		return FReply::Unhandled();
	}

	const FString AndroidCommandLineFile = FString::Printf(TEXT("/mnt/sdcard/UE4Game/%s/UE4CommandLine.txt"), *FString(FApp::GetProjectName()));
	const FString AdbArguments = FString::Printf(TEXT("push \"%s\" \"%s\""), *OutCommandLineArgsFile, *AndroidCommandLineFile);

	TryPushCommandLineArgsToDevice(AdbExe, AdbArguments, OutCommandLineArgsFile);
	return FReply::Handled();
}

FReply FSpatialGDKEditorCommandLineArgsManager::RemoveFromIOSDevice()
{
	const UIOSRuntimeSettings* IOSRuntimeSettings = GetDefault<UIOSRuntimeSettings>();

	FString Executable = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::EngineDir(), TEXT("Binaries/DotNET/IOS/deploymentserver.exe")));
	FString DeploymentServerArguments = FString::Printf(TEXT("removefile -bundle \"%s\" -file \"/Documents/ue4commandline.txt\""), *(IOSRuntimeSettings->BundleIdentifier.Replace(TEXT("[PROJECT_NAME]"), FApp::GetProjectName())));

#if PLATFORM_MAC
	DeploymentServerArguments = FString::Printf(TEXT("%s %s"), *Executable, *DeploymentServerArguments);
	Executable = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::EngineDir(), TEXT("Binaries/ThirdParty/Mono/Mac/bin/mono")));
#endif

	FString ExeOutput;
	FString StdErr;
	int32 ExitCode;

	FPlatformProcess::ExecProcess(*Executable, *DeploymentServerArguments, &ExitCode, &ExeOutput, &StdErr);
	if (ExitCode != 0)
	{
		UE_LOG(LogSpatialGDKEditorCommandLineArgsManager, Error, TEXT("Failed to remove settings from the mobile client. %s %s"), *ExeOutput, *StdErr);
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Failed to remove settings from the mobile client. See the Output log for more information.")));
		return FReply::Unhandled();
	}
	UE_LOG(LogSpatialGDKEditorCommandLineArgsManager, Log, TEXT("Remove ue4commandline.txt from the iOS device. %s %s"), *ExeOutput, *StdErr);

	return FReply::Handled();
}

FReply FSpatialGDKEditorCommandLineArgsManager::RemoveFromAndroidDevice()
{
	const FString AdbExe = GetAdbExePath();
	if (AdbExe.IsEmpty())
	{
		return FReply::Unhandled();
	}

	FString ExeOutput;
	FString StdErr;
	int32 ExitCode;

	FString ExeArguments = FString::Printf(TEXT("shell rm -f /mnt/sdcard/UE4Game/%s/UE4CommandLine.txt"), FApp::GetProjectName());

	FPlatformProcess::ExecProcess(*AdbExe, *ExeArguments, &ExitCode, &ExeOutput, &StdErr);
	if (ExitCode != 0)
	{
		UE_LOG(LogSpatialGDKEditorCommandLineArgsManager, Error, TEXT("Failed to remove settings from the mobile client. %s %s"), *ExeOutput, *StdErr);
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Failed to remove settings from the mobile client. See the Output log for more information.")));
		return FReply::Unhandled();
	}
	UE_LOG(LogSpatialGDKEditorCommandLineArgsManager, Log, TEXT("Remove ue4commandline.txt from the Android device. %s %s"), *ExeOutput, *StdErr);

	return FReply::Handled();
}

bool FSpatialGDKEditorCommandLineArgsManager::TryConstructMobileCommandLineArgumentsFile(FString& CommandLineArgsFile)
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	const FString ProjectName = FApp::GetProjectName();

	// The project path is based on this: https://github.com/improbableio/UnrealEngine/blob/4.22-SpatialOSUnrealGDK-release/Engine/Source/Programs/AutomationTool/AutomationUtils/DeploymentContext.cs#L408
	const FString MobileProjectPath = FString::Printf(TEXT("../../../%s/%s.uproject"), *ProjectName, *ProjectName);
	FString TravelUrl;
	FString SpatialOSOptions = FString::Printf(TEXT("-workerType %s"), *(SpatialGDKSettings->MobileWorkerType));
	if (SpatialGDKSettings->bMobileConnectToLocalDeployment)
	{
		if (SpatialGDKSettings->MobileRuntimeIP.IsEmpty())
		{
			UE_LOG(LogSpatialGDKEditorCommandLineArgsManager, Error, TEXT("The Runtime IP is currently not set. Please make sure to specify a Runtime IP."));
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString::Printf(TEXT("The Runtime IP is currently not set. Please make sure to specify a Runtime IP."))));
			return false;
		}

		TravelUrl = SpatialGDKSettings->MobileRuntimeIP;
	}
	else
	{
		TravelUrl = TEXT("connect.to.spatialos");

		if (SpatialGDKSettings->DevelopmentAuthenticationToken.IsEmpty())
		{
			FReply GeneratedTokenReply = GenerateDevAuthToken();
			if (!GeneratedTokenReply.IsEventHandled())
			{
				return false;
			}
		}

		SpatialOSOptions += FString::Printf(TEXT(" +devauthToken %s"), *(SpatialGDKSettings->DevelopmentAuthenticationToken));
		if (!SpatialGDKSettings->DevelopmentDeploymentToConnect.IsEmpty())
		{
			SpatialOSOptions += FString::Printf(TEXT(" +deployment %s"), *(SpatialGDKSettings->DevelopmentDeploymentToConnect));
		}
	}

	const FString SpatialOSCommandLineArgs = FString::Printf(TEXT("%s %s %s %s"), *MobileProjectPath, *TravelUrl, *SpatialOSOptions, *(SpatialGDKSettings->MobileExtraCommandLineArgs));
	CommandLineArgsFile = FPaths::ConvertRelativePathToFull(FPaths::Combine(*FPaths::ProjectLogDir(), TEXT("ue4commandline.txt")));

	if (!FFileHelper::SaveStringToFile(SpatialOSCommandLineArgs, *CommandLineArgsFile, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		UE_LOG(LogSpatialGDKEditorCommandLineArgsManager, Error, TEXT("Failed to write command line args to file: %s"), *CommandLineArgsFile);
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString::Printf(TEXT("Failed to write command line args to file: %s"), *CommandLineArgsFile)));
		return false;
	}

	return true;
}

FReply FSpatialGDKEditorCommandLineArgsManager::GenerateDevAuthToken()
{
	FString DevAuthToken, ErrorMessage;
	if (!SpatialCommandUtils::GenerateDevAuthToken(GetMutableDefault<USpatialGDKSettings>()->IsRunningInChina(), DevAuthToken, ErrorMessage))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(ErrorMessage));
		return FReply::Unhandled();
	}
	if (USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetMutableDefault<USpatialGDKEditorSettings>())
	{
		SpatialGDKEditorSettings->DevelopmentAuthenticationToken = DevAuthToken;
		SpatialGDKEditorSettings->SaveConfig();
		SpatialGDKEditorSettings->SetRuntimeDevelopmentAuthenticationToken();
	}
	return FReply::Handled();
}

bool FSpatialGDKEditorCommandLineArgsManager::TryPushCommandLineArgsToDevice(const FString& Executable, const FString& ExeArguments, const FString& CommandLineArgsFile)
{
	FString ExeOutput;
	FString StdErr;
	int32 ExitCode;

	FPlatformProcess::ExecProcess(*Executable, *ExeArguments, &ExitCode, &ExeOutput, &StdErr);
	if (ExitCode != 0)
	{
		UE_LOG(LogSpatialGDKEditorCommandLineArgsManager, Error, TEXT("Failed to update the mobile client. %s %s"), *ExeOutput, *StdErr);
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Failed to update the mobile client. See the Output log for more information.")));
		return false;
	}

	UE_LOG(LogSpatialGDKEditorCommandLineArgsManager, Log, TEXT("Successfully stored command line args on device: %s"), *ExeOutput);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DeleteFile(*CommandLineArgsFile))
	{
		UE_LOG(LogSpatialGDKEditorCommandLineArgsManager, Error, TEXT("Failed to delete file %s"), *CommandLineArgsFile);
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString::Printf(TEXT("Failed to delete file %s"), *CommandLineArgsFile)));
		return false;
	}

	return true;
}
