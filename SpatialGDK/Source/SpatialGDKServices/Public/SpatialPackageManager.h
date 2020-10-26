// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Async/Future.h"
#include "CoreMinimal.h"
#include "Misc/MonitoredProcess.h"
#include "TimerManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialPackageManager, Log, All);

class SPATIALGDKSERVICES_API FSpatialPackageManager
{
public:
	FSpatialPackageManager();

	static void FetchRuntimeBinary(FString RuntimeVersion);
	static void FetchInspectorBinary(FString InspectorVersion);
	void StartProcess(FString Params, FString ProcessName);
	bool KillProcess(FString ProcessName);

private:
	TOptional<FMonitoredProcess> FetchingProcess = {};
	static const int32 ExitCodeSuccess = 0;
};
