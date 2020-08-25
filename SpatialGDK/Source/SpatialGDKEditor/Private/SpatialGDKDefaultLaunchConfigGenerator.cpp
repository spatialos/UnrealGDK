// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKDefaultLaunchConfigGenerator.h"

#include "EngineClasses/SpatialWorldSettings.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "SpatialGDKEditorModule.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKSettings.h"

#include "Editor.h"
#include "ISettingsModule.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Serialization/JsonWriter.h"
#include "Settings/LevelEditorPlaySettings.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKDefaultLaunchConfigGenerator);

#define LOCTEXT_NAMESPACE "SpatialGDKDefaultLaunchConfigGenerator"

using namespace SpatialGDK;

namespace
{
bool WriteFlagSection(TSharedRef<TJsonWriter<>> Writer, const FString& Key, const FString& Value)
{
	Writer->WriteObjectStart();
	Writer->WriteValue(TEXT("name"), Key);
	Writer->WriteValue(TEXT("value"), Value);
	Writer->WriteObjectEnd();

	return true;
}

bool WriteWorkerSection(TSharedRef<TJsonWriter<>> Writer, const FName& WorkerTypeName, const FWorkerTypeLaunchSection& WorkerConfig)
{
	Writer->WriteObjectStart();
	Writer->WriteValue(TEXT("worker_type"), *WorkerTypeName.ToString());
	Writer->WriteArrayStart(TEXT("flags"));
	for (const auto& Flag : WorkerConfig.Flags)
	{
		WriteFlagSection(Writer, Flag.Key, Flag.Value);
	}
	Writer->WriteArrayEnd();
	Writer->WriteArrayStart(TEXT("permissions"));
	Writer->WriteObjectStart();
	if (WorkerConfig.WorkerPermissions.bAllPermissions)
	{
		Writer->WriteObjectStart(TEXT("all"));
		Writer->WriteObjectEnd();
	}
	else
	{
		Writer->WriteObjectStart(TEXT("entity_creation"));
		Writer->WriteValue(TEXT("allow"), WorkerConfig.WorkerPermissions.bAllowEntityCreation);
		Writer->WriteObjectEnd();
		Writer->WriteObjectStart(TEXT("entity_deletion"));
		Writer->WriteValue(TEXT("allow"), WorkerConfig.WorkerPermissions.bAllowEntityDeletion);
		Writer->WriteObjectEnd();
		Writer->WriteObjectStart(TEXT("entity_query"));
		Writer->WriteValue(TEXT("allow"), WorkerConfig.WorkerPermissions.bAllowEntityQuery);
		Writer->WriteArrayStart("components");
		for (const FString& Component : WorkerConfig.WorkerPermissions.Components)
		{
			Writer->WriteValue(Component);
		}
		Writer->WriteArrayEnd();
		Writer->WriteObjectEnd();
	}
	Writer->WriteObjectEnd();
	Writer->WriteArrayEnd();
	Writer->WriteObjectEnd();

	return true;
}

bool WriteLoadbalancingSection(TSharedRef<TJsonWriter<>> Writer, const FName& WorkerType, uint32 NumEditorInstances,
							   const bool ManualWorkerConnectionOnly)
{
	Writer->WriteObjectStart();
	Writer->WriteValue(TEXT("layer"), *WorkerType.ToString());
	Writer->WriteObjectStart("rectangle_grid");
	Writer->WriteValue(TEXT("cols"), 1);
	Writer->WriteValue(TEXT("rows"), (int32)NumEditorInstances);
	Writer->WriteObjectEnd();
	Writer->WriteObjectStart(TEXT("options"));
	Writer->WriteValue(TEXT("manual_worker_connection_only"), ManualWorkerConnectionOnly);
	Writer->WriteObjectEnd();
	Writer->WriteObjectEnd();

	return true;
}

} // anonymous namespace

uint32 GetWorkerCountFromWorldSettings(const UWorld& World)
{
	const ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(World.GetWorldSettings());
	if (WorldSettings == nullptr)
	{
		UE_LOG(LogSpatialGDKDefaultLaunchConfigGenerator, Error, TEXT("Missing SpatialWorldSettings on map %s"), *World.GetMapName());
		return 1;
	}

	const bool bIsMultiWorkerEnabled = USpatialStatics::IsSpatialMultiWorkerEnabled(&World);
	if (!bIsMultiWorkerEnabled)
	{
		return 1;
	}

	return WorldSettings->MultiWorkerSettingsClass->GetDefaultObject<UAbstractSpatialMultiWorkerSettings>()
		->GetMinimumRequiredWorkerCount();
}

bool FillWorkerConfigurationFromCurrentMap(FWorkerTypeLaunchSection& OutWorker, FIntPoint& OutWorldDimensions)
{
	if (GEditor == nullptr || GEditor->GetWorldContexts().Num() == 0)
	{
		return false;
	}

	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
	const USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetDefault<USpatialGDKEditorSettings>();

	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
	check(EditorWorld != nullptr);

	OutWorker = SpatialGDKEditorSettings->LaunchConfigDesc.ServerWorkerConfig;
	OutWorker.NumEditorInstances = GetWorkerCountFromWorldSettings(*EditorWorld);

	return true;
}

bool GenerateLaunchConfig(const FString& LaunchConfigPath, const FSpatialLaunchConfigDescription* InLaunchConfigDescription,
						  const FWorkerTypeLaunchSection& InWorker)
{
	if (InLaunchConfigDescription != nullptr)
	{
		const FSpatialLaunchConfigDescription& LaunchConfigDescription = *InLaunchConfigDescription;

		FString Text;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Text);

		// Populate json file for launch config
		Writer->WriteObjectStart();													 // Start of json
		Writer->WriteValue(TEXT("template"), LaunchConfigDescription.GetTemplate()); // Template section
		Writer->WriteObjectStart(TEXT("world"));									 // World section begin
		Writer->WriteObjectStart(TEXT("dimensions"));
		Writer->WriteValue(TEXT("x_meters"), LaunchConfigDescription.World.Dimensions.X);
		Writer->WriteValue(TEXT("z_meters"), LaunchConfigDescription.World.Dimensions.Y);
		Writer->WriteObjectEnd();
		Writer->WriteValue(TEXT("chunk_edge_length_meters"), LaunchConfigDescription.World.ChunkEdgeLengthMeters);
		Writer->WriteArrayStart(TEXT("legacy_flags"));
		for (auto& Flag : LaunchConfigDescription.World.LegacyFlags)
		{
			WriteFlagSection(Writer, Flag.Key, Flag.Value);
		}
		Writer->WriteArrayEnd();
		Writer->WriteArrayStart(TEXT("legacy_javaparams"));
		for (auto& Parameter : LaunchConfigDescription.World.LegacyJavaParams)
		{
			WriteFlagSection(Writer, Parameter.Key, Parameter.Value);
		}
		Writer->WriteArrayEnd();
		Writer->WriteObjectStart(TEXT("snapshots"));
		Writer->WriteValue(TEXT("snapshot_write_period_seconds"), LaunchConfigDescription.World.SnapshotWritePeriodSeconds);
		Writer->WriteObjectEnd();
		Writer->WriteObjectEnd();						  // World section end
		Writer->WriteObjectStart(TEXT("load_balancing")); // Load balancing section begin
		Writer->WriteArrayStart("layer_configurations");
		if (InWorker.NumEditorInstances > 0)
		{
			WriteLoadbalancingSection(Writer, SpatialConstants::DefaultServerWorkerType, InWorker.NumEditorInstances,
									  InWorker.bManualWorkerConnectionOnly);
		}
		Writer->WriteArrayEnd();
		Writer->WriteObjectEnd();				  // Load balancing section end
		Writer->WriteArrayStart(TEXT("workers")); // Workers section begin
		if (InWorker.NumEditorInstances > 0)
		{
			WriteWorkerSection(Writer, SpatialConstants::DefaultServerWorkerType, InWorker);
		}
		// Write the client worker section
		FWorkerTypeLaunchSection ClientWorker;
		ClientWorker.WorkerPermissions.bAllPermissions = true;
		WriteWorkerSection(Writer, SpatialConstants::DefaultClientWorkerType, ClientWorker);
		Writer->WriteArrayEnd();  // Worker section end
		Writer->WriteObjectEnd(); // End of json

		Writer->Close();

		if (!FFileHelper::SaveStringToFile(Text, *LaunchConfigPath))
		{
			UE_LOG(LogSpatialGDKDefaultLaunchConfigGenerator, Error,
				   TEXT("Failed to write output file '%s'. It might be that the file is read-only."), *LaunchConfigPath);
			return false;
		}

		return true;
	}

	return false;
}

bool ValidateGeneratedLaunchConfig(const FSpatialLaunchConfigDescription& LaunchConfigDesc, const FWorkerTypeLaunchSection& InWorker)
{
	const USpatialGDKSettings* SpatialGDKRuntimeSettings = GetDefault<USpatialGDKSettings>();

	if (const FString* EnableChunkInterest = LaunchConfigDesc.World.LegacyFlags.Find(TEXT("enable_chunk_interest")))
	{
		if (*EnableChunkInterest == TEXT("true"))
		{
			const EAppReturnType::Type Result = FMessageDialog::Open(
				EAppMsgType::YesNo,
				LOCTEXT(
					"ChunkInterestNotSupported_Prompt",
					"The legacy flag \"enable_chunk_interest\" is set to true in the generated launch configuration. Chunk interest is not "
					"supported and this flag needs to be set to false.\n\nDo you want to configure your launch config settings now?"));

			if (Result == EAppReturnType::Yes)
			{
				FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer("Project", "SpatialGDKEditor", "Editor Settings");
			}

			return false;
		}
	}
	return true;
}

#undef LOCTEXT_NAMESPACE
