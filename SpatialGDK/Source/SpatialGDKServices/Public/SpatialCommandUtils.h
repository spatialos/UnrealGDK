// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialCommandUtils, Log, All);

class SpatialCommandUtils
{
public:

	SPATIALGDKSERVICES_API static void ExecuteSpatialCommandAndReadOutput(FString Arguments, const FString& DirectoryToRun, FString& OutResult, int32& ExitCode, bool bIsRunningInChina);

	SPATIALGDKSERVICES_API static void ExecuteSpatialCommand(FString Arguments, int32* OutReturnCode, FString* OutStdOut, FString* OutStdEr, bool bIsRunningInChina);

	SPATIALGDKSERVICES_API static FProcHandle CreateSpatialProcess(FString Arguments, bool bLaunchDetached, bool bLaunchHidden, bool bLaunchReallyHidden, uint32* OutProcessID, int32 PriorityModifier, const TCHAR* OptionalWorkingDirectory, void* PipeWriteChild, void * PipeReadChild, bool bIsRunningInChina);

	SPATIALGDKSERVICES_API static bool AttemptSpatialAuth(bool bIsRunningInChina);
};
