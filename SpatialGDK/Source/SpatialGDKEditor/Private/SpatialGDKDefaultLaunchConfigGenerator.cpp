// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKDefaultLaunchConfigGenerator.h"

#include "EngineClasses/SpatialWorldSettings.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "SpatialGDKEditorModule.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKSettings.h"
#include "Utils/SpatialStatics.h"

#include "Editor.h"
#include "GenericPlatform/GenericPlatformFile.h"
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

bool WriteLoadbalancingSection(TSharedRef<TJsonWriter<>> Writer, const FName& WorkerType, uint32 NumEditorInstances,
							   const bool bManualWorkerConnectionOnly)
{
	Writer->WriteObjectStart(TEXT("load_balancing"));
	Writer->WriteObjectStart("rectangle_grid");
	Writer->WriteValue(TEXT("cols"), 1);
	Writer->WriteValue(TEXT("rows"), static_cast<int32>(NumEditorInstances));
	Writer->WriteObjectEnd();
	Writer->WriteValue(TEXT("manual_worker_connection_only"), bManualWorkerConnectionOnly);
	Writer->WriteObjectEnd();

	return true;
}

bool WriteWorkerSection(TSharedRef<TJsonWriter<>> Writer, const FWorkerTypeLaunchSection& WorkerConfig)
{
	Writer->WriteObjectStart();

	Writer->WriteValue(TEXT("worker_type"), *WorkerConfig.WorkerTypeName.ToString());

	Writer->WriteArrayStart(TEXT("flags"));
	for (const auto& Flag : WorkerConfig.Flags)
	{
		WriteFlagSection(Writer, Flag.Key, Flag.Value);
	}
	Writer->WriteArrayEnd();

	Writer->WriteObjectStart(TEXT("permissions"));
	Writer->WriteValue(TEXT("entity_creation"), WorkerConfig.WorkerPermissions.bAllowEntityCreation);
	Writer->WriteValue(TEXT("entity_deletion"), WorkerConfig.WorkerPermissions.bAllowEntityDeletion);
	Writer->WriteValue(TEXT("disconnect_worker"), WorkerConfig.WorkerPermissions.bDisconnectWorker);
	Writer->WriteValue(TEXT("reserve_entity_id"), WorkerConfig.WorkerPermissions.bReserveEntityID);
	Writer->WriteValue(TEXT("entity_query"), WorkerConfig.WorkerPermissions.bAllowEntityQuery);
	Writer->WriteObjectEnd();

	if (WorkerConfig.NumEditorInstances > 0)
	{
		WriteLoadbalancingSection(Writer, SpatialConstants::DefaultServerWorkerType, WorkerConfig.NumEditorInstances,
								  WorkerConfig.bManualWorkerConnectionOnly);
	}

	Writer->WriteObjectEnd();
	return true;
}

} // anonymous namespace

uint32 GetWorkerCountFromWorldSettings(const UWorld& World, bool bForceNonEditorSettings)
{
	const ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(World.GetWorldSettings());
	if (WorldSettings == nullptr)
	{
		UE_LOG(LogSpatialGDKDefaultLaunchConfigGenerator, Error, TEXT("Missing SpatialWorldSettings on map %s"), *World.GetMapName());
		return 1;
	}

	return USpatialStatics::GetSpatialMultiWorkerClass(&World, bForceNonEditorSettings)
		->GetDefaultObject<UAbstractSpatialMultiWorkerSettings>()
		->GetMinimumRequiredWorkerCount();
}

bool GenerateLaunchConfig(const FString& LaunchConfigPath, const FSpatialLaunchConfigDescription* InLaunchConfigDescription,
						  bool bGenerateCloudConfig)
{
	if (InLaunchConfigDescription != nullptr)
	{
		const FSpatialLaunchConfigDescription& LaunchConfigDescription = *InLaunchConfigDescription;

		FString Text;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Text);

		// Populate json file for launch config
		Writer->WriteObjectStart();
		Writer->WriteArrayStart(TEXT("runtime_flags"));
		for (const auto& Flag : LaunchConfigDescription.RuntimeFlags)
		{
			WriteFlagSection(Writer, Flag.Key, Flag.Value);
		}
		Writer->WriteArrayEnd();

		Writer->WriteArrayStart(TEXT("workers")); // Workers section begin

		// Write the default server worker config (UnrealWorker)
		WriteWorkerSection(Writer, InLaunchConfigDescription->ServerWorkerConfiguration);

		// Write the client worker section (UnrealClient)
		FWorkerTypeLaunchSection ClientWorker;
		ClientWorker.NumEditorInstances = 0;
		ClientWorker.WorkerTypeName = SpatialConstants::DefaultClientWorkerType;
		WriteWorkerSection(Writer, ClientWorker);

		// For cloud configs we always add the SimulatedPlayerCoordinator and DeploymentManager.
		if (bGenerateCloudConfig)
		{
			// Write the Simulated Player Coordinator section
			FWorkerTypeLaunchSection SimulatedPlayerCoordinator;
			SimulatedPlayerCoordinator.NumEditorInstances = 0;
			SimulatedPlayerCoordinator.WorkerTypeName = TEXT("SimulatedPlayerCoordinator");
			WriteWorkerSection(Writer, SimulatedPlayerCoordinator);

			// Write the Deployment Manager section
			FWorkerTypeLaunchSection DeploymentManagerConfig;
			DeploymentManagerConfig.NumEditorInstances = 0;
			DeploymentManagerConfig.WorkerTypeName = TEXT("DeploymentManager");
			WriteWorkerSection(Writer, DeploymentManagerConfig);
		}

		// Write any additional worker configs that may have been added.
		for (const FWorkerTypeLaunchSection& AdditionalWorkerConfig : LaunchConfigDescription.AdditionalWorkerConfigs)
		{
			WriteWorkerSection(Writer, AdditionalWorkerConfig);
		}

		Writer->WriteArrayEnd(); // Worker section end

		Writer->WriteObjectStart(TEXT("world_dimensions"));
		Writer->WriteValue(TEXT("x_size"), LaunchConfigDescription.World.Dimensions.X);
		Writer->WriteValue(TEXT("z_size"), LaunchConfigDescription.World.Dimensions.Y);
		Writer->WriteObjectEnd(); // World section end

		Writer->WriteValue(TEXT("max_concurrent_workers"), LaunchConfigDescription.MaxConcurrentWorkers);

		Writer->WriteObjectEnd(); // End of json

		Writer->Close();

		if (!FFileHelper::SaveStringToFile(Text, *LaunchConfigPath))
		{
			UE_LOG(LogSpatialGDKDefaultLaunchConfigGenerator, Error,
				   TEXT("Failed to write output file '%s'. It might be that the file is read-only."), *LaunchConfigPath);
			return false;
		}

		if (bGenerateCloudConfig)
		{
			// TODO: UNR-4471 - Remove classic config conversion when new cloud platform exists.
			return ConvertToClassicConfig(LaunchConfigPath, InLaunchConfigDescription);
		}

		return true;
	}

	return false;
}

bool ConvertToClassicConfig(const FString& LaunchConfigPath, const FSpatialLaunchConfigDescription* InLaunchConfigDescription)
{
	// The new runtime binary handles the config conversion. We just need to pass it the parameters.
	// All `runtime_flags` are converted to `legacy_flags` in the conversion process.
	// `max_concurrent_workers` is also converted to a legacy flag.
	// The output config file is called `launch_config.json` when printing and we must provide a path to a folder for it to be saved to.
	// The output config file will replace the generated standalone config file.
	// We do not export the worker configurations as we already generate these for the user still.

	FString LaunchConfigDir = FPaths::GetPath(LaunchConfigPath);

	// runtime.exe --export-classic-config=classic_config_dir --config=Game\Intermediate\Improbable\Control_Small_LocalLaunchConfig.json
	// --launch-template=w2_r0500_e5 --export-worker-configuration=true
	FString ConversionArgs =
		FString::Printf(TEXT("--export-classic-config=\"%s\" --config=\"%s\" --launch-template=%s --export-worker-configuration=false"),
						*LaunchConfigDir, *LaunchConfigPath, *InLaunchConfigDescription->GetTemplate());

	FString Output;
	int32 ExitCode;

	const USpatialGDKEditorSettings* SpatialGDKEditor = GetDefault<USpatialGDKEditorSettings>();
	FString RuntimePath =
		SpatialGDKServicesConstants::GetRuntimeExecutablePath(SpatialGDKEditor->GetSelectedRuntimeVariantVersion().GetVersionForLocal());

	FSpatialGDKServicesModule::ExecuteAndReadOutput(RuntimePath, ConversionArgs, LaunchConfigDir, Output, ExitCode);

	if (ExitCode != SpatialGDKServicesConstants::ExitCodeSuccess)
	{
		UE_LOG(LogSpatialGDKDefaultLaunchConfigGenerator, Error,
			   TEXT("Failed to convert generated launch config to classic style config for config '%s'. It might "
					"be that the file is read-only. Conversion output: %s"),
			   *LaunchConfigPath, *Output);
		return false;
	}

	UE_LOG(LogSpatialGDKDefaultLaunchConfigGenerator, Verbose,
		   TEXT("Successfully converted generated launch config to classic style config for config '%s'. Conversion output: %s"),
		   *LaunchConfigPath, *Output);

	FString GeneratedClassicConfigFilePath = FPaths::Combine(LaunchConfigDir, TEXT("launch_config.json"));

	IPlatformFile& PlatformFile = IPlatformFile::GetPlatformPhysical();

	bool bSuccess = true;

	// Delete the previously generated config that is not classic format.
	if (FPaths::FileExists(LaunchConfigPath))
	{
		bSuccess = PlatformFile.DeleteFile(*LaunchConfigPath);
	}

	if (!bSuccess)
	{
		UE_LOG(LogSpatialGDKDefaultLaunchConfigGenerator, Error,
			   TEXT("Failed to remove local launch configuration '%s' when converting to cloud launch configuration."), *LaunchConfigPath);
		return bSuccess;
	}

	bSuccess = PlatformFile.MoveFile(*LaunchConfigPath, *GeneratedClassicConfigFilePath);
	if (!bSuccess)
	{
		UE_LOG(LogSpatialGDKDefaultLaunchConfigGenerator, Error,
			   TEXT("Failed to rename converted classic style launch config. From: %s. To: %s"), *GeneratedClassicConfigFilePath,
			   *LaunchConfigPath);
	}

	return bSuccess;
}

bool ValidateGeneratedLaunchConfig(const FSpatialLaunchConfigDescription& LaunchConfigDesc)
{
	const USpatialGDKSettings* SpatialGDKRuntimeSettings = GetDefault<USpatialGDKSettings>();
	if (const FString* EnableChunkInterest = LaunchConfigDesc.RuntimeFlags.Find(TEXT("enable_chunk_interest")))
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
