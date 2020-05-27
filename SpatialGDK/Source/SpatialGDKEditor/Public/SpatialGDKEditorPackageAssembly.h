// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Notifications/SNotificationList.h"

#include "CloudDeploymentConfiguration.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKEditorPackageAssembly, Log, All);

class FMonitoredProcess;

class SPATIALGDKEDITOR_API FSpatialGDKPackageAssembly : public TSharedFromThis<FSpatialGDKPackageAssembly>
{
public:
	bool CanBuild() const;

	void BuildAndUploadAssembly(const FCloudDeploymentConfiguration& InCloudDeploymentConfiguration);

	FSimpleDelegate OnSuccess;

private:
	enum class EPackageAssemblyStep
	{
		NONE = 0,
		BUILD_SERVER,
		BUILD_CLIENT,
		BUILD_SIMULATED_PLAYERS,
		UPLOAD_ASSEMBLY,
	};

	TQueue<EPackageAssemblyStep> Steps;

	TSharedPtr<FMonitoredProcess> PackageAssemblyTask;
	TWeakPtr<SNotificationItem> TaskNotificationPtr;

	FCloudDeploymentConfiguration CloudDeploymentConfiguration;

	void LaunchTask(const FString& Exe, const FString& Args, const FString& WorkingDir);

	void BuildAssembly(const FString& ProjectName, const FString& Platform, const FString& Configuration, const FString& AdditionalArgs);
	void UploadAssembly(const FString& AssemblyName, bool bForceAssemblyOverwrite);

	bool NextStep();

	void ShowTaskStartedNotification(const FString& NotificationText);
	void ShowTaskEndedNotification(const FString& NotificationText, SNotificationItem::ECompletionState CompletionState);
	void HandleCancelButtonClicked();
	void OnTaskCompleted(int32 TaskResult);
	void OnTaskOutput(FString Message);
	void OnTaskCanceled();
};
