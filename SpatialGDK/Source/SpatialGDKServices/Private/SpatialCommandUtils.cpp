// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialCommandUtils.h"

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

bool SpatialCommandUtils::GenerateDevAuthToken(bool bIsRunningInChina, FString& OutTokenSecret, FString& OutErrorMessage)
{
	FString Arguments = TEXT("project auth dev-auth-token create --description=\"Unreal GDK Token\" --json_output");
	if (bIsRunningInChina)
	{
		Arguments += SpatialGDKServicesConstants::ChinaEnvironmentArgument;
	}

	FString CreateDevAuthTokenResult;
	int32 ExitCode;
	FSpatialGDKServicesModule::ExecuteAndReadOutput(SpatialGDKServicesConstants::SpatialExe, Arguments, SpatialGDKServicesConstants::SpatialOSDirectory, CreateDevAuthTokenResult, ExitCode);

	if (ExitCode != 0)
	{
		FString ErrorMessage = CreateDevAuthTokenResult;
		TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(CreateDevAuthTokenResult);
		TSharedPtr<FJsonObject> JsonRootObject;
		if (FJsonSerializer::Deserialize(JsonReader, JsonRootObject) && JsonRootObject.IsValid())
		{
			JsonRootObject->TryGetStringField("error", ErrorMessage);
		}
		OutErrorMessage = FString::Printf(TEXT("Unable to generate a development authentication token. Result: %s"), *ErrorMessage);
		return false;
	};

	FString AuthResult;
	FString DevAuthTokenResult;
	bool bFoundNewline = CreateDevAuthTokenResult.TrimEnd().Split(TEXT("\n"), &AuthResult, &DevAuthTokenResult, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	if (!bFoundNewline || DevAuthTokenResult.IsEmpty())
	{
		// This is necessary because spatial might return multiple json structs depending on whether you are already authenticated against spatial and are on the latest version of it.
		DevAuthTokenResult = CreateDevAuthTokenResult;
	}

	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(DevAuthTokenResult);
	TSharedPtr<FJsonObject> JsonRootObject;
	if (!(FJsonSerializer::Deserialize(JsonReader, JsonRootObject) && JsonRootObject.IsValid()))
	{
		OutErrorMessage = FString::Printf(TEXT("Unable to parse the received development authentication token. Result: %s"), *DevAuthTokenResult);
		return false;
	}

	// We need a pointer to a shared pointer due to how the JSON API works.
	const TSharedPtr<FJsonObject>* JsonDataObject;
	if (!(JsonRootObject->TryGetObjectField("json_data", JsonDataObject)))
	{
		OutErrorMessage = FString::Printf(TEXT("Unable to parse the received json data. Result: %s"), *DevAuthTokenResult);
		return false;
	}

	FString TokenSecret;
	if (!(*JsonDataObject)->TryGetStringField("token_secret", TokenSecret))
	{
		OutErrorMessage = FString::Printf(TEXT("Unable to parse the token_secret field inside the received json data. Result: %s"), *DevAuthTokenResult);
		return false;
	}

	OutTokenSecret = TokenSecret;
	return true;
}

bool SpatialCommandUtils::HasDevLoginTag(const FString& DeploymentName, bool bIsRunningInChina, const FString& DirectoryToRun, FString& OutResult, int32& OutExitCode)
{
	if (DeploymentName.IsEmpty())
	{
		// If we don't specify a deployment name, we will not check if any deployment is running.
		return true;
	}

	FString TagsCommand = FString::Printf(TEXT("project deployment tags list %s --json_output"), *DeploymentName);
	if (bIsRunningInChina)
	{
		TagsCommand += SpatialGDKServicesConstants::ChinaEnvironmentArgument;
	}

	FSpatialGDKServicesModule::ExecuteAndReadOutput(*SpatialGDKServicesConstants::SpatialExe, TagsCommand, DirectoryToRun, OutResult, OutExitCode);
	if (OutExitCode != 0)
	{
		FString ErrorMessage = OutResult;
		TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(OutResult);
		TSharedPtr<FJsonObject> JsonRootObject;
		if (FJsonSerializer::Deserialize(JsonReader, JsonRootObject) && JsonRootObject.IsValid())
		{
			JsonRootObject->TryGetStringField("error", ErrorMessage);
		}
		UE_LOG(LogSpatialCommandUtils, Warning, TEXT("Unable to retrieve deployment tags. Is the deployment %s running? Result: %s"), *DeploymentName, *ErrorMessage);
		return false;
	};

	FString AuthResult;
	FString RetrieveTagsResult;
	bool bFoundNewline = OutResult.TrimEnd().Split(TEXT("\n"), &AuthResult, &RetrieveTagsResult, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	if (!bFoundNewline || RetrieveTagsResult.IsEmpty())
	{
		// This is necessary because spatial might return multiple json structs depending on whether you are already authenticated against spatial and are on the latest version of it.
		RetrieveTagsResult = OutResult;
	}

	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(RetrieveTagsResult);
	TSharedPtr<FJsonObject> JsonRootObject;
	if (!(FJsonSerializer::Deserialize(JsonReader, JsonRootObject) && JsonRootObject.IsValid()))
	{
		UE_LOG(LogSpatialCommandUtils, Error, TEXT("Unable to parse the received tags. Result: %s"), *RetrieveTagsResult);
		return false;
	}


	FString JsonMessage;
	if (!(JsonRootObject)->TryGetStringField("msg", JsonMessage))
	{
		UE_LOG(LogSpatialCommandUtils, Error, TEXT("Unable to parse the msg field inside the received json data. Result: %s"), *RetrieveTagsResult);
		return false;
	}

	/*
	Output looks like this:
	Tags: [unreal_deployment_launcher,dev_login]
	We need to parse it a bit to be able to iterate through the tags
	*/
	FString test = JsonMessage.Mid(7, JsonMessage.Len() - 8);
	TArray<FString> Tags;
	test.ParseIntoArray(Tags, TEXT(","), true);

	for (int i = 0; i < Tags.Num(); i++)
	{
		if (Tags[i] == SpatialGDKServicesConstants::DevLoginDeploymentTag)
		{
			return true;
		}
	}

	UE_LOG(LogSpatialCommandUtils, Error, TEXT("The cloud deployment %s does not have the dev_login tag associated with it. The client won't be able to connect to the deployment"), *DeploymentName);
	return false;
}
