// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorPackageAssembly.h"

#include "Async/Async.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Misc/App.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Misc/MonitoredProcess.h"
#include "UnrealEdMisc.h"

#include "SpatialGDKEditorModule.h"
#include "SpatialGDKServicesConstants.h"
#include "SpatialGDKServicesModule.h"
#include "SpatialGDKSettings.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKEditorPackageAssembly);

#define LOCTEXT_NAMESPACE "SpatialGDKEditorPackageAssembly"

namespace
{
const FString SpatialBuildExe =
	FSpatialGDKServicesModule::GetSpatialGDKPluginDirectory(TEXT("SpatialGDK/Binaries/ThirdParty/Improbable/Programs/Build.exe"));
const FString LinuxPlatform = TEXT("Linux");
const FString Win64Platform = TEXT("Win64");
} // anonymous namespace

void FSpatialGDKPackageAssembly::LaunchTask(const FString& Exe, const FString& Args, const FString& WorkingDir)
{
	PackageAssemblyTask = MakeShareable(new FMonitoredProcess(Exe, Args, WorkingDir, /* Hidden */ true));
	PackageAssemblyTask->OnCompleted().BindSP(this, &FSpatialGDKPackageAssembly::OnTaskCompleted);
	PackageAssemblyTask->OnOutput().BindSP(this, &FSpatialGDKPackageAssembly::OnTaskOutput);
	PackageAssemblyTask->OnCanceled().BindSP(this, &FSpatialGDKPackageAssembly::OnTaskCanceled);
	PackageAssemblyTask->Launch();
}

void FSpatialGDKPackageAssembly::BuildAssembly(const FString& ProjectName, const FString& Platform, const FString& Configuration,
											   const FString& AdditionalArgs)
{
	FString WorkingDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	FString Project = FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());
	FString Args = FString::Printf(TEXT("%s %s %s \"%s\" %s"), *ProjectName, *Platform, *Configuration, *Project, *AdditionalArgs);
	LaunchTask(SpatialBuildExe, Args, WorkingDir);
}

void FSpatialGDKPackageAssembly::UploadAssembly(const FString& AssemblyName, bool bForceAssemblyOverwrite)
{
	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
	const FString& WorkingDir = SpatialGDKServicesConstants::SpatialOSDirectory;
	FString Flags = TEXT("--no_animation");
	if (bForceAssemblyOverwrite)
	{
		Flags += TEXT(" --force");
	}
	if (SpatialGDKSettings->IsRunningInChina())
	{
		Flags += SpatialGDKServicesConstants::ChinaEnvironmentArgument;
	}
	FString Args = FString::Printf(TEXT("cloud upload %s %s"), *AssemblyName, *Flags);
	LaunchTask(SpatialGDKServicesConstants::SpatialExe, Args, WorkingDir);
}

void FSpatialGDKPackageAssembly::BuildAndUploadAssembly(const FCloudDeploymentConfiguration& InCloudDeploymentConfiguration)
{
	if (CanBuild())
	{
		Status = EPackageAssemblyStatus::NONE;
		CloudDeploymentConfiguration = InCloudDeploymentConfiguration;

		Steps.Enqueue(EPackageAssemblyStep::BUILD_SERVER);
		if (CloudDeploymentConfiguration.bBuildClientWorker)
		{
			Steps.Enqueue(EPackageAssemblyStep::BUILD_CLIENT);
		}
		if (CloudDeploymentConfiguration.bSimulatedPlayersEnabled)
		{
			Steps.Enqueue(EPackageAssemblyStep::BUILD_SIMULATED_PLAYERS);
		}
		Steps.Enqueue(EPackageAssemblyStep::UPLOAD_ASSEMBLY);

		AsyncTask(ENamedThreads::GameThread, [this]() {
			ShowTaskStartedNotification(TEXT("Building Assembly"));
			NextStep();
		});
	}
}

bool FSpatialGDKPackageAssembly::CanBuild() const
{
	return Steps.IsEmpty();
}

bool FSpatialGDKPackageAssembly::NextStep()
{
	bool bHasStepsRemaining = false;
	EPackageAssemblyStep Target = EPackageAssemblyStep::NONE;
	if (Steps.Dequeue(Target))
	{
		bHasStepsRemaining = true;
		switch (Target)
		{
		case EPackageAssemblyStep::BUILD_SERVER:
			AsyncTask(ENamedThreads::GameThread, [this]() {
				BuildAssembly(FString::Printf(TEXT("%sServer"), FApp::GetProjectName()), LinuxPlatform,
							  CloudDeploymentConfiguration.BuildConfiguration, CloudDeploymentConfiguration.BuildServerExtraArgs);
			});
			break;
		case EPackageAssemblyStep::BUILD_CLIENT:
			AsyncTask(ENamedThreads::GameThread, [this]() {
				BuildAssembly(FApp::GetProjectName(), Win64Platform, CloudDeploymentConfiguration.BuildConfiguration,
							  CloudDeploymentConfiguration.BuildClientExtraArgs);
			});
			break;
		case EPackageAssemblyStep::BUILD_SIMULATED_PLAYERS:
			AsyncTask(ENamedThreads::GameThread, [this]() {
				BuildAssembly(FString::Printf(TEXT("%sSimulatedPlayer"), FApp::GetProjectName()), LinuxPlatform,
							  CloudDeploymentConfiguration.BuildConfiguration, CloudDeploymentConfiguration.BuildSimulatedPlayerExtraArgs);
			});
			break;
		case EPackageAssemblyStep::UPLOAD_ASSEMBLY:
			AsyncTask(ENamedThreads::GameThread, [this]() {
				UploadAssembly(CloudDeploymentConfiguration.AssemblyName, CloudDeploymentConfiguration.bForceAssemblyOverwrite);
			});
			break;
		default:
			checkNoEntry();
		}
	}
	return bHasStepsRemaining;
}

void FSpatialGDKPackageAssembly::OnTaskCompleted(int32 TaskResult)
{
	if (TaskResult == 0)
	{
		if (!NextStep())
		{
			AsyncTask(ENamedThreads::GameThread, [this]() {
				FString NotificationMessage =
					FString::Printf(TEXT("Assembly successfully uploaded to project: %s"), *FSpatialGDKServicesModule::GetProjectName());
				ShowTaskEndedNotification(NotificationMessage, SNotificationItem::CS_Success);
				OnSuccess.ExecuteIfBound();
			});
		}
	}
	else
	{
		AsyncTask(ENamedThreads::GameThread, [this]() {
			FString NotificationMessage =
				FString::Printf(TEXT("Failed assembly upload to project: %s"), *FSpatialGDKServicesModule::GetProjectName());
			ShowTaskEndedNotification(NotificationMessage, SNotificationItem::CS_Fail);
			if (Status == EPackageAssemblyStatus::ASSEMBLY_EXISTS)
			{
				FMessageDialog::Open(
					EAppMsgType::Ok,
					LOCTEXT(
						"AssemblyExists_Error",
						"The assembly with the specified name has previously been uploaded. Enable the 'Force Overwrite on Upload' option "
						"in the Cloud Deployment dialog to overwrite the existing assembly or specify a different assembly name."));
			}
			else if (Status == EPackageAssemblyStatus::BAD_PROJECT_NAME)
			{
				FMessageDialog::Open(EAppMsgType::Ok,
									 LOCTEXT("BadProjectName_Error",
											 "The project name appears to be incorrect or you do not have permissions for this project. "
											 "You can edit the project name from the Cloud Deployment dialog."));
			}
			else if (Status == EPackageAssemblyStatus::NONE)
			{
				Status = EPackageAssemblyStatus::UNKNOWN_ERROR;
			}
		});
		Steps.Empty();
	}
}

void FSpatialGDKPackageAssembly::OnTaskOutput(FString Message)
{
	// UNR-3486 parse for assembly name conflict so we can display a message to the user
	// because the spatial cli doesn't return error codes this is done via string matching
	if (Message.Find(TEXT("Either change the name or use the '--force' flag")) >= 0)
	{
		Status = EPackageAssemblyStatus::ASSEMBLY_EXISTS;
	}
	else if (Message.Find(TEXT("Make sure the project name is correct and you have permission to upload new assemblies")) >= 0)
	{
		Status = EPackageAssemblyStatus::BAD_PROJECT_NAME;
	}
	UE_LOG(LogSpatialGDKEditorPackageAssembly, Display, TEXT("%s"), *Message);
}

void FSpatialGDKPackageAssembly::OnTaskCanceled()
{
	Steps.Empty();
	Status = EPackageAssemblyStatus::CANCELED;
	FString NotificationMessage =
		FString::Printf(TEXT("Cancelled assembly upload to project: %s"), *FSpatialGDKServicesModule::GetProjectName());
	AsyncTask(ENamedThreads::GameThread, [this, NotificationMessage]() {
		ShowTaskEndedNotification(NotificationMessage, SNotificationItem::CS_Fail);
	});
}

void FSpatialGDKPackageAssembly::HandleCancelButtonClicked()
{
	if (PackageAssemblyTask.IsValid())
	{
		PackageAssemblyTask->Cancel(true);
	}
}

void FSpatialGDKPackageAssembly::ShowTaskStartedNotification(const FString& NotificationText)
{
	FNotificationInfo Info(FText::AsCultureInvariant(NotificationText));
	Info.ButtonDetails.Add(FNotificationButtonInfo(
		LOCTEXT("PackageAssemblyTaskCancel", "Cancel"), LOCTEXT("PackageAssemblyTaskCancel_ToolTip", "Cancels execution of this task."),
		FSimpleDelegate::CreateRaw(this, &FSpatialGDKPackageAssembly::HandleCancelButtonClicked), SNotificationItem::CS_Pending));
	Info.ExpireDuration = 5.0f;
	Info.bFireAndForget = false;

	TaskNotificationPtr = FSlateNotificationManager::Get().AddNotification(Info);

	if (TaskNotificationPtr.IsValid())
	{
		TaskNotificationPtr.Pin()->SetCompletionState(SNotificationItem::CS_Pending);
	}
}

void FSpatialGDKPackageAssembly::ShowTaskEndedNotification(const FString& NotificationText,
														   SNotificationItem::ECompletionState CompletionState)
{
	TSharedPtr<SNotificationItem> Notification = TaskNotificationPtr.Pin();
	if (Notification.IsValid())
	{
		Notification->SetFadeInDuration(0.1f);
		Notification->SetFadeOutDuration(0.5f);
		Notification->SetExpireDuration(5.0);
		Notification->SetText(FText::AsCultureInvariant(NotificationText));
		Notification->SetCompletionState(CompletionState);
		Notification->ExpireAndFadeout();
	}
}

#undef LOCTEXT_NAMESPACE
