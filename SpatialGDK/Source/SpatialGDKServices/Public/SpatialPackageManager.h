// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "CoreMinimal.h"
#include "Misc/MonitoredProcess.h"
#include "TimerManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialPackageManager, Log, All);

class SPATIALGDKSERVICES_API FSpatialPackageManager
{
public:
	FSpatialPackageManager();

	static void FetchRuntimeBinary(const FString& RuntimeVersion);
	static void FetchInspectorBinary(const FString& InspectorVersion);
	void StartProcess(const FString& Params, const FString& ProcessName);
	bool KillProcess(const FString& ProcessName);

private:
	TOptional<FMonitoredProcess> FetchingProcess;
	static const int32 ExitCodeSuccess = 0;
};
