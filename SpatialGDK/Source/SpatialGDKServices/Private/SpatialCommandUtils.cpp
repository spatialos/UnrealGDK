// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialCommandUtils.h"
#include "SpatialGDKServicesConstants.h"
#include "SpatialGDKServicesModule.h"

DEFINE_LOG_CATEGORY(LogSpatialCommandUtils);

namespace
{
	FString ChinaEnvironmentArgument = TEXT(" --environment=cn-production");
} // anonymous namespace

bool SpatialCommandUtils::SpatialVersion(bool bIsRunningInChina, const FString& DirectoryToRun, FString& OutResult, int32& OutExitCode)
{
	FString Command = TEXT("version");

	if (bIsRunningInChina)
	{
		Command += ChinaEnvironmentArgument;
	}

	FSpatialGDKServicesModule::ExecuteAndReadOutput(*SpatialGDKServicesConstants::SpatialExe, Command, DirectoryToRun, OutResult, OutExitCode);

	bool bSuccess = OutExitCode == 0;
	if (!bSuccess)
	{
		UE_LOG(LogSpatialCommandUtils, Warning, TEXT("Spatial version failed. Error Code: %d, Error Message: %s"), OutExitCode, *OutResult);
	}

	return bSuccess;
}

bool SpatialCommandUtils::AttemptSpatialAuth(bool bIsRunningInChina)
{
	FString Command = TEXT("auth login");

	if (bIsRunningInChina)
	{
		Command += ChinaEnvironmentArgument;
	}

	int32 OutExitCode;
	FString OutStdOut;
	FString OutStdErr;

	FPlatformProcess::ExecProcess(*SpatialGDKServicesConstants::SpatialExe, *Command, &OutExitCode, &OutStdOut, &OutStdErr);

	bool bSuccess = OutExitCode == 0;
	if (!bSuccess)
	{
		UE_LOG(LogSpatialCommandUtils, Warning, TEXT("Spatial auth login failed. Error Code: %d, StdOut Message: %s, StdErr Message: %s"), OutExitCode, *OutStdOut, *OutStdErr);
	}

	return bSuccess;
}

bool SpatialCommandUtils::StartSpatialService(const FString& Version, const FString& RuntimeIP, bool bIsRunningInChina, const FString& DirectoryToRun, FString& OutResult, int32& OutExitCode)
{
	FString Command = TEXT("service start");

	if (bIsRunningInChina)
	{
		Command += ChinaEnvironmentArgument;
	}

	if (!Version.IsEmpty())
	{
		Command.Append(FString::Printf(TEXT(" --version=%s"), *Version));
	}

	if (!RuntimeIP.IsEmpty())
	{
		Command.Append(FString::Printf(TEXT(" --runtime_ip=%s"), *RuntimeIP));
		UE_LOG(LogSpatialCommandUtils, Verbose, TEXT("Trying to start spatial service with exposed runtime ip: %s"), *RuntimeIP);
	}

	FSpatialGDKServicesModule::ExecuteAndReadOutput(*SpatialGDKServicesConstants::SpatialExe, Command, DirectoryToRun, OutResult, OutExitCode);

	bool bSuccess = OutExitCode == 0;
	if (!bSuccess)
	{
		UE_LOG(LogSpatialCommandUtils, Warning, TEXT("Spatial start service failed. Error Code: %d, Error Message: %s"), OutExitCode, *OutResult);
	}

	return bSuccess;
}

bool SpatialCommandUtils::StopSpatialService(bool bIsRunningInChina, const FString& DirectoryToRun, FString& OutResult, int32& OutExitCode)
{
	FString Command = TEXT("service stop");

	if (bIsRunningInChina)
	{
		Command += ChinaEnvironmentArgument;
	}

	FSpatialGDKServicesModule::ExecuteAndReadOutput(*SpatialGDKServicesConstants::SpatialExe, Command, DirectoryToRun, OutResult, OutExitCode);

	bool bSuccess = OutExitCode == 0;
	if (!bSuccess)
	{
		UE_LOG(LogSpatialCommandUtils, Warning, TEXT("Spatial stop service failed. Error Code: %d, Error Message: %s"), OutExitCode, *OutResult);
	}

	return bSuccess;
}

bool SpatialCommandUtils::BuildWorkerConfig(bool bIsRunningInChina, const FString& DirectoryToRun, FString& OutResult, int32& OutExitCode)
{
	FString Command = TEXT("worker build build-config");

	if (bIsRunningInChina)
	{
		Command += ChinaEnvironmentArgument;
	}

	FSpatialGDKServicesModule::ExecuteAndReadOutput(*SpatialGDKServicesConstants::SpatialExe, Command, DirectoryToRun, OutResult, OutExitCode);

	bool bSuccess = OutExitCode == 0;
	if (!bSuccess)
	{
		UE_LOG(LogSpatialCommandUtils, Warning, TEXT("Spatial build worker config failed. Error Code: %d, Error Message: %s"), OutExitCode, *OutResult);
	}

	return bSuccess;
}

FProcHandle SpatialCommandUtils::LocalWorkerReplace(const FString& ServicePort, const FString& OldWorker, const FString& NewWorker, bool bIsRunningInChina, uint32* OutProcessID)
{
	check(!ServicePort.IsEmpty());
	check(!OldWorker.IsEmpty());
	check(!NewWorker.IsEmpty());

	FString Command = TEXT("worker build build-config");
	Command.Append(FString::Printf(TEXT(" --local_service_grpc_port %s"), *ServicePort));
	Command.Append(FString::Printf(TEXT(" --existing_worker_id %s"), *OldWorker));
	Command.Append(FString::Printf(TEXT(" --replacing_worker_id %s"), *NewWorker));

	return FPlatformProcess::CreateProc(*SpatialGDKServicesConstants::SpatialExe, *Command, false, true, true, OutProcessID, 2 /*PriorityModifier*/,
		nullptr, nullptr, nullptr);
}
