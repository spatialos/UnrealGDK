// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKDefaultLaunchConfigGenerator.h"

#include "EngineClasses/SpatialWorldSettings.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "SpatialGDKEditorModule.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKSettings.h"
#include "Utils/SpatialStatics.h"

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

bool WriteLoadbalancingSection(TSharedRef<TJsonWriter<>> Writer, const FName& WorkerType, uint32 NumEditorInstances,
							   const bool ManualWorkerConnectionOnly)
{
	Writer->WriteObjectStart(TEXT("load_balancing"));
	Writer->WriteObjectStart("rectangle_grid");
	Writer->WriteValue(TEXT("cols"), 1);
	Writer->WriteValue(TEXT("rows"), (int32)NumEditorInstances);
	Writer->WriteObjectEnd();
	Writer->WriteValue(TEXT("manual_worker_connection_only"), ManualWorkerConnectionOnly);
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

	Writer->WriteObjectStart(TEXT("permissions"));
	Writer->WriteValue(TEXT("entity_creation"), WorkerConfig.WorkerPermissions.bAllowEntityCreation);
	Writer->WriteValue(TEXT("entity_deletion"), WorkerConfig.WorkerPermissions.bAllowEntityDeletion);
	Writer->WriteValue(TEXT("disconnect_worker"), WorkerConfig.WorkerPermissions.bDisconnectWorker);
	Writer->WriteValue(TEXT("reserve_entity_id"), WorkerConfig.WorkerPermissions.bReserveEntityID);
	Writer->WriteValue(TEXT("entity_query"), WorkerConfig.WorkerPermissions.bAllowEntityQuery);
	Writer->WriteObjectEnd();

	Writer->WriteValue(TEXT("attribute"), WorkerTypeName.ToString());

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
						  const FWorkerTypeLaunchSection& InWorker)
{
	if (InLaunchConfigDescription != nullptr)
	{
		const FSpatialLaunchConfigDescription& LaunchConfigDescription = *InLaunchConfigDescription;

		FString Text;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Text);

		// Populate json file for launch config
		Writer->WriteObjectStart();
		Writer->WriteArrayStart(TEXT("runtime_flags"));
		for (auto& Flag : LaunchConfigDescription.RuntimeFlags)
		{
			WriteFlagSection(Writer, Flag.Key, Flag.Value);
		}
		Writer->WriteArrayEnd();

		Writer->WriteArrayStart(TEXT("workers")); // Workers section begin
		if (InWorker.NumEditorInstances > 0)
		{
			WriteWorkerSection(Writer, SpatialConstants::DefaultServerWorkerType, InWorker);
		}
		// Write the client worker section
		FWorkerTypeLaunchSection ClientWorker;
		ClientWorker.NumEditorInstances = 0; // TODO: This is probably incorrect.
		WriteWorkerSection(Writer, SpatialConstants::DefaultClientWorkerType, ClientWorker);

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

		return true;
	}

	return false;
}

bool ValidateGeneratedLaunchConfig(const FSpatialLaunchConfigDescription& LaunchConfigDesc, const FWorkerTypeLaunchSection& InWorker)
{
	const USpatialGDKSettings* SpatialGDKRuntimeSettings = GetDefault<USpatialGDKSettings>();

	// if (const FString* EnableChunkInterest = LaunchConfigDesc.World.RuntimeFlags.Find(TEXT("enable_chunk_interest")))
	//{
	//	if (*EnableChunkInterest == TEXT("true"))
	//	{
	//		const EAppReturnType::Type Result = FMessageDialog::Open(
	//			EAppMsgType::YesNo,
	//			LOCTEXT(
	//				"ChunkInterestNotSupported_Prompt",
	//				"The legacy flag \"enable_chunk_interest\" is set to true in the generated launch configuration. Chunk interest is not "
	//				"supported and this flag needs to be set to false.\n\nDo you want to configure your launch config settings now?"));

	//		if (Result == EAppReturnType::Yes)
	//		{
	//			FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer("Project", "SpatialGDKEditor", "Editor Settings");
	//		}

	//		return false;
	//	}
	//}
	return true;
}

#undef LOCTEXT_NAMESPACE
