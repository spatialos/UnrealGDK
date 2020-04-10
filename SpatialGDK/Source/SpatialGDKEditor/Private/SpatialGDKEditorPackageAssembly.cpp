// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorPackageAssembly.h"

#include "Async/Async.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Misc/App.h"
#include "Misc/FileHelper.h"
#include "SpatialGDKEditorModule.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKServicesConstants.h"
#include "SpatialGDKServicesModule.h"
#include "SpatialGDKSettings.h"
#include "UnrealEdMisc.h"


DEFINE_LOG_CATEGORY(LogSpatialGDKEditorPackageAssembly);

#define LOCTEXT_NAMESPACE "SpatialGDKEditorPackageAssembly"

namespace
{
	const FString SpatialBuildExe = FSpatialGDKServicesModule::GetSpatialGDKPluginDirectory(TEXT("SpatialGDK/Binaries/ThirdParty/Improbable/Programs/Build.exe"));
	const FString Linux = TEXT("Linux");
}


FSpatialGDKPackageAssembly::FSpatialGDKPackageAssembly() : CurrentAssemblyTarget{ EPackageAssemblyTarget::NONE }
{

}

static FString GetStagingDir()
{
	return FPaths::ConvertRelativePathToFull((FPaths::IsProjectFilePathSet() ? FPaths::GetPath(FPaths::GetProjectFilePath()) : FPaths::RootDir() / FApp::GetProjectName()) / TEXT("..") / TEXT("spatial") / TEXT("build") / TEXT("unreal"));
}

void FSpatialGDKPackageAssembly::BuildAssembly(const FString& ProjectName, const FString& Platform, const FString& Configuration, const FString& AdditionalArgs)
{
	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
	FString WorkingDir = FPaths::ConvertRelativePathToFull(FPaths::IsProjectFilePathSet() ? FPaths::GetPath(FPaths::GetProjectFilePath()) : FPaths::RootDir() / FApp::GetProjectName());
	FString Project = FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());
	FString Args = FString::Printf(TEXT("%s %s %s %s"), *ProjectName, *Platform, *Configuration, *Project, *AdditionalArgs);
	PackageAssemblyTask = MakeShareable(new FMonitoredProcess(SpatialBuildExe, Args, WorkingDir, true));
	PackageAssemblyTask->OnCompleted().BindRaw(this, &FSpatialGDKPackageAssembly::OnTaskCompleted);
	PackageAssemblyTask->OnOutput().BindRaw(this, &FSpatialGDKPackageAssembly::OnTaskOutput);
	PackageAssemblyTask->OnCanceled().BindRaw(this, &FSpatialGDKPackageAssembly::OnTaskCanceled);
	PackageAssemblyTask->Launch();
	FString NotificationMessage = FString::Printf(TEXT("Building %s Assembly"), *ProjectName);
}

void FSpatialGDKPackageAssembly::UploadAssembly(const FString &AssemblyName, bool Force)
{
	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
	FString WorkingDir = FPaths::ConvertRelativePathToFull((FPaths::IsProjectFilePathSet() ? FPaths::GetPath(FPaths::GetProjectFilePath()) : FPaths::RootDir() / FApp::GetProjectName()) / TEXT("..") / TEXT("spatial"));
	FString Args = FString::Printf(TEXT("cloud upload %s %s %s --no_animation"), *AssemblyName, Force ? TEXT("--force") : TEXT(""), SpatialGDKSettings->IsRunningInChina() ? TEXT("--environment=cn-production") : TEXT(""));
	PackageAssemblyTask = MakeShareable(new FMonitoredProcess(SpatialGDKServicesConstants::SpatialExe, Args, WorkingDir, true));
	PackageAssemblyTask->OnCompleted().BindRaw(this, &FSpatialGDKPackageAssembly::OnTaskCompleted);
	PackageAssemblyTask->OnOutput().BindRaw(this, &FSpatialGDKPackageAssembly::OnTaskOutput);
	PackageAssemblyTask->OnCanceled().BindRaw(this, &FSpatialGDKPackageAssembly::OnTaskCanceled);
	PackageAssemblyTask->Launch();
	FString NotificationMessage = FString::Printf(TEXT("Uploading Assembly to Project: %s"), *FSpatialGDKServicesModule::GetProjectName());
	if (AssemblyDetailsPtr.IsValid())
	{
		AssemblyDetailsPtr.Reset(nullptr);
	}
}

void FSpatialGDKPackageAssembly::BuildAllAndUpload(const FString& AssemblyName, const FString& WindowsPlatform, const FString& Configuration, const FString& AdditionalArgs, bool Force)
{
	if (AssemblyDetailsPtr == nullptr && CurrentAssemblyTarget == EPackageAssemblyTarget::NONE)
	{
		AssemblyDetailsPtr.Reset(new AssemblyDetails(AssemblyName, Configuration, Force));
		CurrentAssemblyTarget = EPackageAssemblyTarget::BUILD_CLIENT;
		BuildAssembly(FApp::GetProjectName(), WindowsPlatform, Configuration, AdditionalArgs);

		AsyncTask(ENamedThreads::GameThread, [this]() { this->ShowTaskStartedNotification(TEXT("Building Assembly"));	});
	}
}

bool FSpatialGDKPackageAssembly::CanBuild() const
{
	return CurrentAssemblyTarget == EPackageAssemblyTarget::NONE;
}

void FSpatialGDKPackageAssembly::OnTaskCompleted(int32 Result)
{
	if (Result == 0)
	{
		const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
		switch (CurrentAssemblyTarget)
		{
		case EPackageAssemblyTarget::BUILD_CLIENT:
			CurrentAssemblyTarget = EPackageAssemblyTarget::BUILD_SERVER;
			AsyncTask(ENamedThreads::GameThread, [this]() {
				this->BuildAssembly(FString::Printf(TEXT("%sServer"), FApp::GetProjectName()), Linux, AssemblyDetailsPtr->Configuration, TEXT(""));
				});
			break;
		case EPackageAssemblyTarget::BUILD_SERVER:
			if (SpatialGDKSettings->IsSimulatedPlayersEnabled())
			{
				CurrentAssemblyTarget = EPackageAssemblyTarget::BUILD_SIMULATED_PLAYERS;
				AsyncTask(ENamedThreads::GameThread, [this]() {
					this->BuildAssembly(FString::Printf(TEXT("%sSimulatedPlayer"), FApp::GetProjectName()), Linux, AssemblyDetailsPtr->Configuration, TEXT(""));
					});
			}
			else
			{
				CurrentAssemblyTarget = EPackageAssemblyTarget::UPLOAD_ASSEMBLY;
				AsyncTask(ENamedThreads::GameThread, [this]() {
					this->AssemblyDetailsPtr->Upload(*this);
					});
			}
			break;
		case EPackageAssemblyTarget::BUILD_SIMULATED_PLAYERS:
			CurrentAssemblyTarget = EPackageAssemblyTarget::UPLOAD_ASSEMBLY;
			AsyncTask(ENamedThreads::GameThread, [this]() {
				this->AssemblyDetailsPtr->Upload(*this);
				});
			break;
		case EPackageAssemblyTarget::UPLOAD_ASSEMBLY:
			{
				FString NotificationMessage = FString::Printf(TEXT("Assembly Successfully uploaded to Project: %s"), *FSpatialGDKServicesModule::GetProjectName());
				AsyncTask(ENamedThreads::GameThread, [this]() { this->ShowTaskEndedNotification(TEXT("Assembly Failed"), SNotificationItem::CS_Success);	});
			}
			break;
		default:
			CurrentAssemblyTarget = EPackageAssemblyTarget::NONE;
			;
		}
	}
	else
	{
		FString NotificationMessage = FString::Printf(TEXT("Failed Assembly Upload to Project: %s"), *FSpatialGDKServicesModule::GetProjectName());
		AsyncTask(ENamedThreads::GameThread, [this]() { this->ShowTaskEndedNotification(TEXT("Assembly Failed"), SNotificationItem::CS_Fail);	});
		CurrentAssemblyTarget = EPackageAssemblyTarget::NONE;
	}
}

void FSpatialGDKPackageAssembly::OnTaskOutput(FString Output)
{
	UE_LOG(LogSpatialGDKEditorPackageAssembly, Display, TEXT("%s"), *Output);
}

void FSpatialGDKPackageAssembly::OnTaskCanceled()
{
	CurrentAssemblyTarget = EPackageAssemblyTarget::NONE;
	FString NotificationMessage = FString::Printf(TEXT("Canceled Assembly Upload to Project: %s"), *FSpatialGDKServicesModule::GetProjectName());
	AsyncTask(ENamedThreads::GameThread, [this, NotificationMessage]() { this->ShowTaskEndedNotification(NotificationMessage, SNotificationItem::CS_Fail); this->AssemblyDetailsPtr.Reset();	});
}

FSpatialGDKPackageAssembly::AssemblyDetails::AssemblyDetails(const FString& Name, const FString& Config, bool Force) : AssemblyName(Name), Configuration(Config), bForce(Force)
{

}

void FSpatialGDKPackageAssembly::AssemblyDetails::Upload(FSpatialGDKPackageAssembly &PackageAssembly)
{
	PackageAssembly.UploadAssembly(AssemblyName, bForce);
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
	Info.ButtonDetails.Add(
		FNotificationButtonInfo(
			LOCTEXT("PackageAssemblyTaskCancel", "Cancel"),
			LOCTEXT("PackageAssemblyTaskCancelToolTip", "Cancels execution of this task."),
			FSimpleDelegate::CreateRaw(this, &FSpatialGDKPackageAssembly::HandleCancelButtonClicked),
			SNotificationItem::CS_Pending
		)
	);
	Info.ExpireDuration = 5.0f;
	Info.bFireAndForget = false;

	TaskNotificationPtr = FSlateNotificationManager::Get().AddNotification(Info);

	if (TaskNotificationPtr.IsValid())
	{
		TaskNotificationPtr.Pin()->SetCompletionState(SNotificationItem::CS_Pending);
	}
}

void FSpatialGDKPackageAssembly::ShowTaskEndedNotification(const FString& NotificationText, SNotificationItem::ECompletionState CompletionState)
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
