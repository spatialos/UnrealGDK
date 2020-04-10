// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"
#include "Misc/MonitoredProcess.h"
#include "Widgets/Notifications/SNotificationList.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKEditorPackageAssembly, Log, All);

class SPATIALGDKEDITOR_API FSpatialGDKPackageAssembly : public TSharedFromThis<FSpatialGDKPackageAssembly>
{
public:
	FSpatialGDKPackageAssembly();

	bool CanBuild() const;

	void BuildAllAndUpload(const FString &AssemblyName, const FString& WindowsPlatform, const FString& Configuration, const FString& AdditionalArgs, bool Force);
	
private:
	enum class EPackageAssemblyTarget
	{
		NONE = 0,
		START_ALL,
		BUILD_CLIENT,
		BUILD_SERVER,
		BUILD_SIMULATED_PLAYERS,
		UPLOAD_ASSEMBLY,
	} CurrentAssemblyTarget;

	TSharedPtr<FMonitoredProcess> PackageAssemblyTask;
	TWeakPtr<SNotificationItem> TaskNotificationPtr;

	struct AssemblyDetails
	{
		FString AssemblyName;
		FString Configuration;
		bool bForce;
		AssemblyDetails(const FString& Name, const FString& Config, bool Force);
		void Upload(FSpatialGDKPackageAssembly& PackageAssembly);
	};

	TUniquePtr<AssemblyDetails> AssemblyDetailsPtr;

	void BuildAssembly(const FString& ProjectName, const FString& Platform, const FString& Configuration, const FString& AdditionalArgs);
	void UploadAssembly(const FString& AssemblyName, bool Force);

	void ShowTaskStartedNotification(const FString& NotificationText);
	void ShowTaskEndedNotification(const FString& NotificationText, SNotificationItem::ECompletionState CompletionState);
	void HandleCancelButtonClicked();
	void OnTaskCompleted(int32);
	void OnTaskOutput(FString);
	void OnTaskCanceled();
};
