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

FSpatialGDKPackageAssembly::FSpatialGDKPackageAssembly()
{
}

static FString GetStagingDir()
{
	return SpatialGDKServicesConstants::SpatialOSDirectory / TEXT("build/unreal");
}

void FSpatialGDKPackageAssembly::BuildAssembly(const FString& ProjectName, const FString& Platform, const FString& Configuration, const FString& AdditionalArgs)
{
	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
	FString WorkingDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	FString Project = FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());
	FString Args = FString::Printf(TEXT("%s %s %s %s %s"), *ProjectName, *Platform, *Configuration, *Project, *AdditionalArgs);
	PackageAssemblyTask = MakeShareable(new FMonitoredProcess(SpatialBuildExe, Args, WorkingDir, true));
	PackageAssemblyTask->OnCompleted().BindSP(this, &FSpatialGDKPackageAssembly::OnTaskCompleted);
	PackageAssemblyTask->OnOutput().BindSP(this, &FSpatialGDKPackageAssembly::OnTaskOutput);
	PackageAssemblyTask->OnCanceled().BindSP(this, &FSpatialGDKPackageAssembly::OnTaskCanceled);
	PackageAssemblyTask->Launch();
	FString NotificationMessage = FString::Printf(TEXT("Building %s Assembly"), *ProjectName);
}

void FSpatialGDKPackageAssembly::UploadAssembly(const FString& AssemblyName, bool bForce)
{
	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
	FString WorkingDir = SpatialGDKServicesConstants::SpatialOSDirectory;
	FString Flags = TEXT("--no_animation");
	if (bForce)
	{
		Flags += TEXT(" --force");
	}
	if (SpatialGDKSettings->IsRunningInChina())
	{
		Flags += TEXT(" --environment=cn-production");
	}
	FString Args = FString::Printf(TEXT("cloud upload %s %s"), *AssemblyName, *Flags);
	PackageAssemblyTask = MakeShareable(new FMonitoredProcess(SpatialGDKServicesConstants::SpatialExe, Args, WorkingDir, true));
	PackageAssemblyTask->OnCompleted().BindSP(this, &FSpatialGDKPackageAssembly::OnTaskCompleted);
	PackageAssemblyTask->OnOutput().BindSP(this, &FSpatialGDKPackageAssembly::OnTaskOutput);
	PackageAssemblyTask->OnCanceled().BindSP(this, &FSpatialGDKPackageAssembly::OnTaskCanceled);
	PackageAssemblyTask->Launch();
	FString NotificationMessage = FString::Printf(TEXT("Uploading Assembly to Project: %s"), *FSpatialGDKServicesModule::GetProjectName());
	if (AssemblyDetailsPtr.IsValid())
	{
		AssemblyDetailsPtr.Reset();
	}
}

void FSpatialGDKPackageAssembly::BuildAllAndUpload(const FString& AssemblyName, const FString& WindowsPlatform, const FString& Configuration, const FString& AdditionalArgs, bool bForce)
{
	if (AssemblyDetailsPtr == nullptr && Steps.IsEmpty())
	{
		AssemblyDetailsPtr.Reset(new AssemblyDetails(AssemblyName, WindowsPlatform, Configuration, bForce));
		const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();

		Steps.Enqueue(EPackageAssemblyStep::BUILD_SERVER);
		if (SpatialGDKSettings->IsBuildClientWorkerEnabled())
		{
			Steps.Enqueue(EPackageAssemblyStep::BUILD_CLIENT);
		}
		if (SpatialGDKSettings->IsSimulatedPlayersEnabled())
		{
			Steps.Enqueue(EPackageAssemblyStep::BUILD_SIMULATED_PLAYERS);
		}
		Steps.Enqueue(EPackageAssemblyStep::UPLOAD_ASSEMBLY);

		AsyncTask(ENamedThreads::GameThread, [this]()
		{
			this->ShowTaskStartedNotification(TEXT("Building Assembly"));
			this->NextStep();
		});
	}
}

bool FSpatialGDKPackageAssembly::CanBuild() const
{
	return Steps.IsEmpty();
}

bool FSpatialGDKPackageAssembly::NextStep()
{
	bool HasMoreSteps = false;
	EPackageAssemblyStep Target = EPackageAssemblyStep::NONE;
	if (Steps.Dequeue(Target))
	{
		HasMoreSteps = true;
		switch(Target)
		{
		case EPackageAssemblyStep::BUILD_SERVER:
			AsyncTask(ENamedThreads::GameThread, [this]()
			{
				this->BuildAssembly(FString::Printf(TEXT("%sServer"), FApp::GetProjectName()), Linux, AssemblyDetailsPtr->Configuration, TEXT(""));
			});
			break;
		case EPackageAssemblyStep::BUILD_CLIENT:
			AsyncTask(ENamedThreads::GameThread, [this]()
			{
				this->BuildAssembly(FApp::GetProjectName(), AssemblyDetailsPtr->WindowsPlatform, AssemblyDetailsPtr->Configuration, TEXT(""));
			});
			break;
		case EPackageAssemblyStep::BUILD_SIMULATED_PLAYERS:
			AsyncTask(ENamedThreads::GameThread, [this]()
			{
				this->BuildAssembly(FString::Printf(TEXT("%sSimulatedPlayer"), FApp::GetProjectName()), Linux, AssemblyDetailsPtr->Configuration, TEXT(""));
			});
			break;
		case EPackageAssemblyStep::UPLOAD_ASSEMBLY:
		{
			AsyncTask(ENamedThreads::GameThread, [this]()
			{
				this->AssemblyDetailsPtr->Upload(*this);
			});
		}
		break;
		}
	}
	return HasMoreSteps;
}

void FSpatialGDKPackageAssembly::OnTaskCompleted(int32 TaskResult)
{
	if (TaskResult == 0)
	{
		if (!NextStep())
		{
			AsyncTask(ENamedThreads::GameThread, [this]()
			{
				FString NotificationMessage = FString::Printf(TEXT("Assembly successfully uploaded to project: %s"), *FSpatialGDKServicesModule::GetProjectName());
				this->ShowTaskEndedNotification(NotificationMessage, SNotificationItem::CS_Success);
				OnSuccess.ExecuteIfBound();
			});
		}
	}
	else
	{
		FString NotificationMessage = FString::Printf(TEXT("Failed assembly upload to project: %s"), *FSpatialGDKServicesModule::GetProjectName());
		AsyncTask(ENamedThreads::GameThread, [this]() { this->ShowTaskEndedNotification(TEXT("Assembly Failed"), SNotificationItem::CS_Fail); });
		Steps.Empty();
	}
}

void FSpatialGDKPackageAssembly::OnTaskOutput(FString Message)
{
	UE_LOG(LogSpatialGDKEditorPackageAssembly, Display, TEXT("%s"), *Message);
}

void FSpatialGDKPackageAssembly::OnTaskCanceled()
{
	Steps.Empty();
	FString NotificationMessage = FString::Printf(TEXT("Cancelled assembly upload to project: %s"), *FSpatialGDKServicesModule::GetProjectName());
	AsyncTask(ENamedThreads::GameThread, [this, NotificationMessage]()
	{
		this->ShowTaskEndedNotification(NotificationMessage, SNotificationItem::CS_Fail);
		this->AssemblyDetailsPtr.Reset();
	});
}

FSpatialGDKPackageAssembly::AssemblyDetails::AssemblyDetails(const FString& Name, const FString& WinPlat, const FString& Config, bool bInForce)
	: AssemblyName(Name)
	, WindowsPlatform(WinPlat)
	, Configuration(Config)
	, bForce(bInForce)
{
}

void FSpatialGDKPackageAssembly::AssemblyDetails::Upload(FSpatialGDKPackageAssembly& PackageAssembly)
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
