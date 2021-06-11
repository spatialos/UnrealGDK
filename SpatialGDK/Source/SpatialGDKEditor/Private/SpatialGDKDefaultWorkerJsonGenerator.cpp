// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKDefaultWorkerJsonGenerator.h"

#include "SpatialGDKServicesConstants.h"
#include "SpatialGDKSettings.h"

#include "Misc/FileHelper.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKDefaultWorkerJsonGenerator);
#define LOCTEXT_NAMESPACE "SpatialGDKDefaultWorkerJsonGenerator"

bool GenerateDefaultWorkerJson(const FString& JsonPath, bool& bOutRedeployRequired)
{
	const FString TemplateWorkerJsonPath =
		FSpatialGDKServicesModule::GetSpatialGDKPluginDirectory(TEXT("SpatialGDK/Extras/templates/WorkerJsonTemplate.json"));

	FString Contents;
	if (FFileHelper::LoadFileToString(Contents, *TemplateWorkerJsonPath))
	{
		if (FFileHelper::SaveStringToFile(Contents, *JsonPath))
		{
			bOutRedeployRequired = true;
			UE_LOG(LogSpatialGDKDefaultWorkerJsonGenerator, Verbose, TEXT("Wrote default worker json to %s"), *JsonPath)

			return true;
		}
		else
		{
			UE_LOG(LogSpatialGDKDefaultWorkerJsonGenerator, Error, TEXT("Failed to write default worker json to %s"), *JsonPath)
		}
	}
	else
	{
		UE_LOG(LogSpatialGDKDefaultWorkerJsonGenerator, Error, TEXT("Failed to read default worker json template at %s"),
			   *TemplateWorkerJsonPath)
	}

	return false;
}

bool GenerateAllDefaultWorkerJsons(bool& bOutRedeployRequired)
{
	const FString WorkerJsonDir = FPaths::Combine(SpatialGDKServicesConstants::SpatialOSDirectory, TEXT("workers/unreal"));
	bool bAllJsonsGeneratedSuccessfully = true;

	if (const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>())
	{
		// Create an array of worker types with a bool signifying whether the associated worker json should exist or not.
		// Then correct the file system state so it matches our expectations.
		TArray<TPair<FName, bool>> WorkerTypes;
		WorkerTypes.Add(TPair<FName, bool>(SpatialConstants::DefaultServerWorkerType, true));
		const bool bRoutingWorkerEnabled = SpatialGDKSettings->CrossServerRPCImplementation == ECrossServerRPCImplementation::RoutingWorker;
		WorkerTypes.Add(TPair<FName, bool>(SpatialConstants::RoutingWorkerType, bRoutingWorkerEnabled));
		WorkerTypes.Add(TPair<FName, bool>(SpatialConstants::StrategyWorkerType, SpatialGDKSettings->bRunStrategyWorker));

		for (const auto& Pair : WorkerTypes)
		{
			FString JsonPath = FPaths::Combine(WorkerJsonDir, FString::Printf(TEXT("spatialos.%s.worker.json"), *Pair.Key.ToString()));
			const bool bFileExists = FPaths::FileExists(JsonPath);
			if (!bFileExists && Pair.Value)
			{
				UE_LOG(LogSpatialGDKDefaultWorkerJsonGenerator, Verbose, TEXT("Could not find worker json at %s"), *JsonPath);

				if (!GenerateDefaultWorkerJson(JsonPath, bOutRedeployRequired))
				{
					bAllJsonsGeneratedSuccessfully = false;
				}
			}
			if (bFileExists && !Pair.Value)
			{
				UE_LOG(LogSpatialGDKDefaultWorkerJsonGenerator, Verbose, TEXT("Found worker json at %s"), *JsonPath);

				IFileManager& FileManager = IFileManager::Get();
				if (!FileManager.Delete(*JsonPath))
				{
					UE_LOG(LogSpatialGDKDefaultWorkerJsonGenerator, Verbose, TEXT("Failed to delete default worker json from %s"),
						   *JsonPath);
					bAllJsonsGeneratedSuccessfully = false;
				}
				else
				{
					bOutRedeployRequired = true;
					UE_LOG(LogSpatialGDKDefaultWorkerJsonGenerator, Verbose, TEXT("Deleted default worker json from %s"), *JsonPath);
				}
			}
		}

		return bAllJsonsGeneratedSuccessfully;
	}

	return false;
}

#undef LOCTEXT_NAMESPACE
