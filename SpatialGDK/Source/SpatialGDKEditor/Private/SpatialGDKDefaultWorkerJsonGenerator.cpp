// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKDefaultWorkerJsonGenerator.h"

#include "SpatialGDKEditorSettings.h"

#include "Misc/FileHelper.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKDefaultWorkerJsonGenerator);
#define LOCTEXT_NAMESPACE "SpatialGDKDefaultWorkerJsonGenerator"

bool GenerateDefaultWorkerJson(bool& bOutRedeployRequired)
{
	if (const USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetDefault<USpatialGDKEditorSettings>())
	{
		const FString WorkerJsonDir = FSpatialGDKServicesModule::GetSpatialOSDirectory(TEXT("workers/unreal"));
		const FString TemplateWorkerJsonPath = FSpatialGDKServicesModule::GetSpatialGDKPluginDirectory(TEXT("SpatialGDK/Extras/templates/WorkerJsonTemplate.json"));

		const FSpatialLaunchConfigDescription& LaunchConfigDescription = SpatialGDKEditorSettings->LaunchConfigDesc;
		for (const FWorkerTypeLaunchSection& Worker : LaunchConfigDescription.ServerWorkers)
		{
			FString JsonPath = FPaths::Combine(WorkerJsonDir, FString::Printf(TEXT("spatialos.%s.worker.json"), *Worker.WorkerTypeName.ToString()));
			if (!FPaths::FileExists(JsonPath))
			{
				UE_LOG(LogSpatialGDKDefaultWorkerJsonGenerator, Verbose, TEXT("Could not find worker json at %s"), *JsonPath);
				FString Contents;
				if (FFileHelper::LoadFileToString(Contents, *TemplateWorkerJsonPath))
				{
					Contents.ReplaceInline(TEXT("{{WorkerTypeName}}"), *Worker.WorkerTypeName.ToString());
					if (FFileHelper::SaveStringToFile(Contents, *JsonPath))
					{
						bOutRedeployRequired = true;
						UE_LOG(LogSpatialGDKDefaultWorkerJsonGenerator, Verbose, TEXT("Wrote default worker json to %s"), *JsonPath)
					}
					else
					{
						UE_LOG(LogSpatialGDKDefaultWorkerJsonGenerator, Error, TEXT("Failed to write default worker json to %s"), *JsonPath)
					}
				}
				else
				{
					UE_LOG(LogSpatialGDKDefaultWorkerJsonGenerator, Error, TEXT("Failed to read default worker json template at %s"), *TemplateWorkerJsonPath)
				}
			}
			else
			{
				UE_LOG(LogSpatialGDKDefaultWorkerJsonGenerator, Verbose, TEXT("Found worker json at %s"), *JsonPath)
			}
		}

		return true;
	}

	return false;
}

#undef LOCTEXT_NAMESPACE
