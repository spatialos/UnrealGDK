// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKServicesModule.h"

#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "SpatialGDKServicesPrivate.h"

#define LOCTEXT_NAMESPACE "FSpatialGDKServicesModule"

DEFINE_LOG_CATEGORY(LogSpatialGDKServices);

IMPLEMENT_MODULE(FSpatialGDKServicesModule, SpatialGDKServices);


void FSpatialGDKServicesModule::StartupModule()
{
}

void FSpatialGDKServicesModule::ShutdownModule()
{
}

FString FSpatialGDKServicesModule::ProjectName = FSpatialGDKServicesModule::ParseProjectName();

FLocalDeploymentManager* FSpatialGDKServicesModule::GetLocalDeploymentManager()
{
	return &LocalDeploymentManager;
}

FString FSpatialGDKServicesModule::GetSpatialOSDirectory(const FString& AppendPath)
{
	return FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT("/../spatial/"), AppendPath));
}

FString FSpatialGDKServicesModule::GetSpatialGDKPluginDirectory(const FString& AppendPath)
{
	FString PluginDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("UnrealGDK")));

	if (!FPaths::DirectoryExists(PluginDir))
	{
		// If the Project Plugin doesn't exist then use the Engine Plugin.
		PluginDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::EnginePluginsDir(), TEXT("UnrealGDK")));
		ensure(FPaths::DirectoryExists(PluginDir));
	}

	return FPaths::ConvertRelativePathToFull(FPaths::Combine(PluginDir, AppendPath));
}

bool FSpatialGDKServicesModule::ParseJson(const FString& RawJsonString, TSharedPtr<FJsonObject>& JsonParsed)
{
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(RawJsonString);
	return FJsonSerializer::Deserialize(JsonReader, JsonParsed);
}

// ExecuteAndReadOutput exists so that a spatial command window does not spawn when using 'spatial.exe'. It does not however allow reading from StdErr.
// For other processes which do not spawn cmd windows, use ExecProcess instead.
void FSpatialGDKServicesModule::ExecuteAndReadOutput(const FString& Executable, const FString& Arguments, const FString& DirectoryToRun, FString& OutResult, int32& ExitCode)
{
	UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("Executing '%s' with arguments '%s' in directory '%s'"), *Executable, *Arguments, *DirectoryToRun);

	void* ReadPipe = nullptr;
	void* WritePipe = nullptr;
	ensure(FPlatformProcess::CreatePipe(ReadPipe, WritePipe));

	FProcHandle ProcHandle = FPlatformProcess::CreateProc(*Executable, *Arguments, false, true, true, nullptr, 1 /*PriorityModifer*/, *DirectoryToRun, WritePipe);

	if (ProcHandle.IsValid())
	{
		for (bool bProcessFinished = false; !bProcessFinished; )
		{
			bProcessFinished = FPlatformProcess::GetProcReturnCode(ProcHandle, &ExitCode);

			OutResult = OutResult.Append(FPlatformProcess::ReadPipe(ReadPipe));
			FPlatformProcess::Sleep(0.01f);
		}

		FPlatformProcess::CloseProc(ProcHandle);
	}
	else
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Execution failed. '%s' with arguments '%s' in directory '%s' "), *Executable, *Arguments, *DirectoryToRun);
	}

	FPlatformProcess::ClosePipe(0, ReadPipe);
	FPlatformProcess::ClosePipe(0, WritePipe);
}

FString FSpatialGDKServicesModule::ParseProjectName()
{
	FString ProjectNameParsed;
	const FString SpatialDirectory = FSpatialGDKServicesModule::GetSpatialOSDirectory();

	FString SpatialFileName = TEXT("spatialos.json");
	FString SpatialFileResult;
	FFileHelper::LoadFileToString(SpatialFileResult, *FPaths::Combine(SpatialDirectory, SpatialFileName));

	TSharedPtr<FJsonObject> JsonParsedSpatialFile;
	if (ParseJson(SpatialFileResult, JsonParsedSpatialFile))
	{
		if (JsonParsedSpatialFile->TryGetStringField(TEXT("name"), ProjectNameParsed))
		{
			return ProjectNameParsed;
		}
		else
		{
			UE_LOG(LogSpatialDeploymentManager, Error, TEXT("'name' does not exist in spatialos.json. Can't read project name."));
		}
	}
	else
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Json parsing of spatialos.json failed. Can't get project name."));
	}

	ProjectNameParsed.Empty();
	return ProjectNameParsed;
}

#undef LOCTEXT_NAMESPACE
