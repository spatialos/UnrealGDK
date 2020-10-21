// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Async/Future.h"
#include "CoreMinimal.h"
#include "FileCache.h"
#include "Improbable/SpatialGDKSettingsBridge.h"
#include "Modules/ModuleManager.h"
#include "Templates/SharedPointer.h"
#include "TimerManager.h"
#include "Misc/MonitoredProcess.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialPackageManager, Log, All);

class SPATIALGDKSERVICES_API FSpatialPackageManager
{
public:
	FSpatialPackageManager();

	void Init();

	static bool TryFetchRuntimeBinary(FString RuntimeVersion);
	static bool TryFetchInspectorBinary(FString InspectorVersion);
	void StartProcess(FString Params, FString ProcessName);
	void KillProcess(FString ProcessName);

private:
	TOptional<FMonitoredProcess> FetchingProcess = {};
	static const int32 ExitCodeSuccess = 0;
	static const int32 ExitCodeNotRunning = 4;
};
