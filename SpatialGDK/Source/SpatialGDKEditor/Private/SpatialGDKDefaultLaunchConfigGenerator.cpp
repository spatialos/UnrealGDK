// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKDefaultLaunchConfigGenerator.h"

#include "EditorExtension/LBStrategyEditorExtension.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "SpatialGDKEditorModule.h"
#include "SpatialGDKSettings.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialRuntimeLoadBalancingStrategies.h"

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
		if (WorkerConfig.MaxConnectionCapacityLimit > 0)
		{
			Writer->WriteObjectStart(TEXT("connection_capacity_limit"));
				Writer->WriteValue(TEXT("max_capacity"), WorkerConfig.MaxConnectionCapacityLimit);
			Writer->WriteObjectEnd();
		}
		if (WorkerConfig.bLoginRateLimitEnabled)
		{
			Writer->WriteObjectStart(TEXT("login_rate_limit"));
				Writer->WriteValue(TEXT("duration"), WorkerConfig.LoginRateLimit.Duration);
				Writer->WriteValue(TEXT("requests_per_duration"), WorkerConfig.LoginRateLimit.RequestsPerDuration);
			Writer->WriteObjectEnd();
		}
	Writer->WriteObjectEnd();

	return true;
}

bool WriteLoadbalancingSection(TSharedRef<TJsonWriter<>> Writer, const FName& WorkerType, UAbstractRuntimeLoadBalancingStrategy& Strategy, const bool ManualWorkerConnectionOnly)
{
	Writer->WriteObjectStart();
		Writer->WriteValue(TEXT("layer"), *WorkerType.ToString());
		Strategy.WriteToConfiguration(Writer);
		Writer->WriteObjectStart(TEXT("options"));
			Writer->WriteValue(TEXT("manual_worker_connection_only"), ManualWorkerConnectionOnly);
		Writer->WriteObjectEnd();
	Writer->WriteObjectEnd();

	return true;
}

} // anonymous namespace

void SetLevelEditorPlaySettingsWorkerType(const FWorkerTypeLaunchSection& InWorker)
{
	ULevelEditorPlaySettings* PlayInSettings = GetMutableDefault<ULevelEditorPlaySettings>();

	PlayInSettings->WorkerTypesToLaunch.Empty(1);

	// TODO: Engine PR to remove PlayInSettings WorkerType map.
	PlayInSettings->WorkerTypesToLaunch.Add(SpatialConstants::DefaultServerWorkerType, InWorker.NumEditorInstances);
}

uint32 GetWorkerCountFromWorldSettings(const UWorld& World)
{
	const ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(World.GetWorldSettings());

	if (WorldSettings == nullptr)
	{
		UE_LOG(LogSpatialGDKDefaultLaunchConfigGenerator, Error, TEXT("Missing SpatialWorldSettings on map %s"), *World.GetMapName());
		return 1;
	}

	if (WorldSettings->bEnableMultiWorker == false)
	{
		return 1;
	}

	FSpatialGDKEditorModule& EditorModule = FModuleManager::GetModuleChecked<FSpatialGDKEditorModule>("SpatialGDKEditor");
	uint32 NumWorkers = 0;
	if (WorldSettings->DefaultLayerLoadBalanceStrategy == nullptr)
	{
		UE_LOG(LogSpatialGDKDefaultLaunchConfigGenerator, Error, TEXT("Missing Load balancing strategy on map %s"), *World.GetMapName());
		return 1;
	}
	else
	{
		UAbstractRuntimeLoadBalancingStrategy* LoadBalancingStrat = nullptr;
		FIntPoint Dimension;
		if (!EditorModule.GetLBStrategyExtensionManager().GetDefaultLaunchConfiguration(WorldSettings->DefaultLayerLoadBalanceStrategy->GetDefaultObject<UAbstractLBStrategy>(), LoadBalancingStrat, Dimension))
		{
			UE_LOG(LogSpatialGDKDefaultLaunchConfigGenerator, Error, TEXT("Could not get the default SpatialOS Load balancing strategy from %s"), *WorldSettings->DefaultLayerLoadBalanceStrategy->GetName());
			NumWorkers += 1;
		}
		else
		{
			NumWorkers += LoadBalancingStrat->GetNumberOfWorkersForPIE();
		}
	}

	for (const auto& Layer : WorldSettings->WorkerLayers)
	{
		const FName& LayerKey = Layer.Key;
		const FLayerInfo& LayerInfo = Layer.Value;

		UAbstractRuntimeLoadBalancingStrategy* LoadBalancingStrat = nullptr;
		FIntPoint Dimension;
		if (!EditorModule.GetLBStrategyExtensionManager().GetDefaultLaunchConfiguration(LayerInfo.LoadBalanceStrategy->GetDefaultObject<UAbstractLBStrategy>(), LoadBalancingStrat, Dimension))
		{
			UE_LOG(LogSpatialGDKDefaultLaunchConfigGenerator, Error, TEXT("Could not get the SpatialOS Load balancing strategy for layer %s"), *LayerKey.ToString());
			NumWorkers += 1;
		}
		else
		{
			NumWorkers += LoadBalancingStrat->GetNumberOfWorkersForPIE();
		}
	}

	return NumWorkers;
}

bool TryGetLoadBalancingStrategyFromWorldSettings(const UWorld& World, UAbstractRuntimeLoadBalancingStrategy*& OutStrategy, FIntPoint& OutWorldDimension)
{
	const ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(World.GetWorldSettings());

	if (WorldSettings == nullptr || !WorldSettings->bEnableMultiWorker)
	{
		UE_LOG(LogSpatialGDKDefaultLaunchConfigGenerator, Log, TEXT("No SpatialWorldSettings on map %s"), *World.GetMapName());
		return false;
	}

	if (WorldSettings->DefaultLayerLoadBalanceStrategy == nullptr)
	{
		UE_LOG(LogSpatialGDKDefaultLaunchConfigGenerator, Error, TEXT("Missing Load balancing strategy on map %s"), *World.GetMapName());
		return false;
	}

	FSpatialGDKEditorModule& EditorModule = FModuleManager::GetModuleChecked<FSpatialGDKEditorModule>("SpatialGDKEditor");

	if (!EditorModule.GetLBStrategyExtensionManager().GetDefaultLaunchConfiguration(WorldSettings->DefaultLayerLoadBalanceStrategy->GetDefaultObject<UAbstractLBStrategy>(), OutStrategy, OutWorldDimension))
	{
		UE_LOG(LogSpatialGDKDefaultLaunchConfigGenerator, Error, TEXT("Could not get the SpatialOS Load balancing strategy from %s"), *WorldSettings->DefaultLayerLoadBalanceStrategy->GetName());
		return false;
	}

	return true;
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

	USingleWorkerRuntimeStrategy* DefaultStrategy = USingleWorkerRuntimeStrategy::StaticClass()->GetDefaultObject<USingleWorkerRuntimeStrategy>();
	UAbstractRuntimeLoadBalancingStrategy* LoadBalancingStrat = DefaultStrategy;
	TryGetLoadBalancingStrategyFromWorldSettings(*EditorWorld, LoadBalancingStrat, OutWorldDimensions);

	OutWorker = SpatialGDKEditorSettings->LaunchConfigDesc.ServerWorkerConfig;
	return true;
}

bool GenerateLaunchConfig(const FString& LaunchConfigPath, const FSpatialLaunchConfigDescription* InLaunchConfigDescription, const FWorkerTypeLaunchSection& InWorker)
{
	if (InLaunchConfigDescription != nullptr)
	{
		const FSpatialLaunchConfigDescription& LaunchConfigDescription = *InLaunchConfigDescription;

		FString Text;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Text);

		// Populate json file for launch config
		Writer->WriteObjectStart(); // Start of json
			Writer->WriteValue(TEXT("template"), LaunchConfigDescription.Template); // Template section
			Writer->WriteObjectStart(TEXT("world")); // World section begin
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
			Writer->WriteObjectEnd(); // World section end
			Writer->WriteObjectStart(TEXT("load_balancing")); // Load balancing section begin
				Writer->WriteArrayStart("layer_configurations");
				if (InWorker.WorkerLoadBalancing != nullptr)
				{
					WriteLoadbalancingSection(Writer, SpatialConstants::DefaultServerWorkerType, *InWorker.WorkerLoadBalancing, InWorker.bManualWorkerConnectionOnly);
				}
				Writer->WriteArrayEnd();
				Writer->WriteObjectEnd(); // Load balancing section end
				Writer->WriteArrayStart(TEXT("workers")); // Workers section begin
				if (InWorker.WorkerLoadBalancing != nullptr)
				{
					WriteWorkerSection(Writer, SpatialConstants::DefaultServerWorkerType, InWorker);
				}
				// Write the client worker section
				FWorkerTypeLaunchSection ClientWorker;
				ClientWorker.WorkerPermissions.bAllPermissions = true;
				ClientWorker.bLoginRateLimitEnabled = false;
				WriteWorkerSection(Writer, SpatialConstants::DefaultClientWorkerType, ClientWorker);
			Writer->WriteArrayEnd(); // Worker section end
		Writer->WriteObjectEnd(); // End of json

		Writer->Close();

		if (!FFileHelper::SaveStringToFile(Text, *LaunchConfigPath))
		{
			UE_LOG(LogSpatialGDKDefaultLaunchConfigGenerator, Error, TEXT("Failed to write output file '%s'. It might be that the file is read-only."), *LaunchConfigPath);
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
			const EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(TEXT("The legacy flag \"enable_chunk_interest\" is set to true in the generated launch configuration. Chunk interest is not supported and this flag needs to be set to false.\n\nDo you want to configure your launch config settings now?")));

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
