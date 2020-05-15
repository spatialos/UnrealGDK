// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialCommandUtils.h"

#include "Logging/LogMacros.h"
#include "Misc/MessageDialog.h"
#include "Serialization/JsonSerializer.h"
#include "SpatialGDKServicesConstants.h"
#include "SpatialGDKServicesModule.h"

DEFINE_LOG_CATEGORY(LogSpatialCommandUtils);

bool SpatialCommandUtils::SpatialVersion(bool bIsRunningInChina, const FString& DirectoryToRun, FString& OutResult, int32& OutExitCode)
{
	FString Command = TEXT("version");

	if (bIsRunningInChina)
	{
		Command += SpatialGDKServicesConstants::ChinaEnvironmentArgument;
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
		Command += SpatialGDKServicesConstants::ChinaEnvironmentArgument;
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
		Command += SpatialGDKServicesConstants::ChinaEnvironmentArgument;
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
		Command += SpatialGDKServicesConstants::ChinaEnvironmentArgument;
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
		Command += SpatialGDKServicesConstants::ChinaEnvironmentArgument;
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

bool SpatialCommandUtils::GenerateDevAuthToken(bool IsRunningInChina, FString& OutTokenSecret)
{
	FString Arguments = TEXT("project auth dev-auth-token create --description=\"Unreal GDK Token\" --json_output");
	if (IsRunningInChina)
	{
		Arguments += TEXT(" --environment cn-production");
	}

	FString CreateDevAuthTokenResult;
	int32 ExitCode;
	FSpatialGDKServicesModule::ExecuteAndReadOutput(SpatialGDKServicesConstants::SpatialExe, Arguments, SpatialGDKServicesConstants::SpatialOSDirectory, CreateDevAuthTokenResult, ExitCode);

	if (ExitCode != 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString::Printf(TEXT("Unable to generate a development authentication token. Result: %s"), *CreateDevAuthTokenResult)));
		return false;
	};

	FString AuthResult;
	FString DevAuthTokenResult;
	bool bFoundNewline = CreateDevAuthTokenResult.TrimEnd().Split(TEXT("\n"), &AuthResult, &DevAuthTokenResult, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	if (!bFoundNewline || DevAuthTokenResult.IsEmpty())
	{
		// This is necessary because depending on whether you are already authenticated against spatial, it will either return two json structs or one.
		DevAuthTokenResult = CreateDevAuthTokenResult;
	}

	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(DevAuthTokenResult);
	TSharedPtr<FJsonObject> JsonRootObject;
	if (!(FJsonSerializer::Deserialize(JsonReader, JsonRootObject) && JsonRootObject.IsValid()))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString::Printf(TEXT("Unable to parse the received development authentication token. Result: %s"), *DevAuthTokenResult)));
		return false;
	}

	// We need a pointer to a shared pointer due to how the JSON API works.
	const TSharedPtr<FJsonObject>* JsonDataObject;
	if (!(JsonRootObject->TryGetObjectField("json_data", JsonDataObject)))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString::Printf(TEXT("Unable to parse the received json data. Result: %s"), *DevAuthTokenResult)));
		return false;
	}

	FString TokenSecret;
	if (!(*JsonDataObject)->TryGetStringField("token_secret", TokenSecret))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString::Printf(TEXT("Unable to parse the token_secret field inside the received json data. Result: %s"), *DevAuthTokenResult)));
		return false;
	}

	OutTokenSecret = TokenSecret;
	return true;
}
