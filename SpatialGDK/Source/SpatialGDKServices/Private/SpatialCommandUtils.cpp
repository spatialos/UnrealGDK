// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialCommandUtils.h"

DEFINE_LOG_CATEGORY(LogSpatialCommandUtils);

bool SpatialCommandUtils::AttemptSpatialAuth(bool IsRunningInChina)
{
	FString SpatialInfoArgs = IsRunningInChina ? TEXT("auth login --environment=cn-production") : TEXT("auth login");
	FString SpatialInfoResult;
	FString StdErr;
	int32 ExitCode;

	const static FString SpatialExe = TEXT("spatial");
	FPlatformProcess::ExecProcess(*SpatialExe, *SpatialInfoArgs, &ExitCode, &SpatialInfoResult, &StdErr);

	bool bSuccess = ExitCode == 0;
	if (!bSuccess)
	{
		UE_LOG(LogSpatialCommandUtils, Warning, TEXT("Spatial auth login failed. Error Code: %d, Error Message: %s"), ExitCode, *SpatialInfoResult);
	}

	return bSuccess;
}
