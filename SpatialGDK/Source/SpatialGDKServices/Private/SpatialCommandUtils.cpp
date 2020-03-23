// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialCommandUtils.h"
#include "SpatialGDKServicesConstants.h"
#include "SpatialGDKServicesModule.h"

DEFINE_LOG_CATEGORY(LogSpatialCommandUtils);

namespace
{
	FString ChinaEnvironmentArgument = TEXT(" --environment=cn-production");
} // anonymous namespace

void SpatialCommandUtils::ExecuteSpatialCommandAndReadOutput(FString Arguments, const FString& DirectoryToRun, FString& OutResult, int32& OutExitCode, bool bIsRunningInChina)
{
	if (bIsRunningInChina)
	{
		Arguments += ChinaEnvironmentArgument;
	}

	FSpatialGDKServicesModule::ExecuteAndReadOutput(*SpatialGDKServicesConstants::SpatialExe, Arguments, DirectoryToRun, OutResult, OutExitCode);
}

void SpatialCommandUtils::ExecuteSpatialCommand(FString Arguments, int32* OutExitCode, FString* OutStdOut, FString* OutStdEr, bool bIsRunningInChina)
{
	if (bIsRunningInChina)
	{
		Arguments += ChinaEnvironmentArgument;
	}
	FPlatformProcess::ExecProcess(*SpatialGDKServicesConstants::SpatialExe, *Arguments, OutExitCode, OutStdOut, OutStdEr);
}

FProcHandle SpatialCommandUtils::CreateSpatialProcess(FString Arguments, bool bLaunchDetached, bool bLaunchHidden, bool bLaunchReallyHidden, uint32* OutProcessID, int32 PriorityModifier, const TCHAR* OptionalWorkingDirectory, void* PipeWriteChild, void * PipeReadChild, bool bIsRunningInChina)
{
	if (bIsRunningInChina)
	{
		Arguments += ChinaEnvironmentArgument;
	}
	return FPlatformProcess::CreateProc(*SpatialGDKServicesConstants::SpatialExe, *Arguments, bLaunchDetached, bLaunchHidden, bLaunchReallyHidden, OutProcessID, PriorityModifier, OptionalWorkingDirectory, PipeWriteChild, PipeReadChild);
}

bool SpatialCommandUtils::AttemptSpatialAuth(bool bIsRunningInChina)
{
	FString SpatialInfoResult;
	FString StdErr;
	int32 ExitCode;
	ExecuteSpatialCommand(TEXT("auth login"), &ExitCode, &SpatialInfoResult, &StdErr, bIsRunningInChina);

	bool bSuccess = ExitCode == 0;
	if (!bSuccess)
	{
		UE_LOG(LogSpatialCommandUtils, Warning, TEXT("Spatial auth login failed. Error Code: %d, Error Message: %s"), ExitCode, *SpatialInfoResult);
	}

	return bSuccess;
}
