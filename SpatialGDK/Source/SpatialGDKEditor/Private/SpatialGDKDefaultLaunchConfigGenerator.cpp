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

void SetLevelEditorPlaySettingsWorkerTypes(const TMap<FName, FWorkerTypeLaunchSection>& InWorkers)
{
	ULevelEditorPlaySettings* PlayInSettings = GetMutableDefault<ULevelEditorPlaySettings>();

	PlayInSettings->WorkerTypesToLaunch.Empty(InWorkers.Num());
	for (const auto& Worker : InWorkers)
	{
		if (Worker.Value.bAutoNumEditorInstances)
		{
			PlayInSettings->WorkerTypesToLaunch.Add(Worker.Key, Worker.Value.WorkerLoadBalancing->GetNumberOfWorkersForPIE());
		}
		else
		{
			PlayInSettings->WorkerTypesToLaunch.Add(Worker.Key, Worker.Value.NumEditorInstances);
		}
	}
}

bool GetLoadBalancingStrategyFromWorldSettings(const UWorld& World, UAbstractRuntimeLoadBalancingStrategy*& OutStrategy, FIntPoint& OutWorldDimension)
{
	const ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(World.GetWorldSettings());

	if (WorldSettings == nullptr)
	{
		UE_LOG(LogSpatialGDKDefaultLaunchConfigGenerator, Error, TEXT("Missing SpatialWorldSettings on map %s"), *World.GetMapName());
		return false;
	}

	if (WorldSettings->LoadBalanceStrategy == nullptr)
	{
		UE_LOG(LogSpatialGDKDefaultLaunchConfigGenerator, Error, TEXT("Missing Load balancing strategy on map %s"), *World.GetMapName());
		return false;
	}

	FSpatialGDKEditorModule& EditorModule = FModuleManager::GetModuleChecked<FSpatialGDKEditorModule>("SpatialGDKEditor");

	if (!EditorModule.GetLBStrategyExtensionManager().GetDefaultLaunchConfiguration(WorldSettings->LoadBalanceStrategy->GetDefaultObject<UAbstractLBStrategy>(), OutStrategy, OutWorldDimension))
	{
		UE_LOG(LogSpatialGDKDefaultLaunchConfigGenerator, Error, TEXT("Could not get the SpatialOS Load balancing strategy from %s"), *WorldSettings->LoadBalanceStrategy->GetName());
		return false;
	}

	return true;
}

bool FillWorkerConfigurationFromCurrentMap(TMap<FName, FWorkerTypeLaunchSection>& OutWorkers, FIntPoint& OutWorldDimensions)
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

	if (SpatialGDKSettings->bEnableUnrealLoadBalancer)
	{
		GetLoadBalancingStrategyFromWorldSettings(*EditorWorld, LoadBalancingStrat, OutWorldDimensions);
	}

	for (const TPair<FName, FWorkerTypeLaunchSection>& WorkerType : SpatialGDKEditorSettings->LaunchConfigDesc.ServerWorkersMap)
	{
		FWorkerTypeLaunchSection Conf = WorkerType.Value;
		Conf.WorkerLoadBalancing = LoadBalancingStrat;
		OutWorkers.Add(WorkerType.Key, Conf);
	}

	return true;
}

bool GenerateLaunchConfig(const FString& LaunchConfigPath, const FSpatialLaunchConfigDescription* InLaunchConfigDescription, const TMap<FName, FWorkerTypeLaunchSection>& InWorkers)
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
				for (const auto& Worker : InWorkers)
				{
					if (Worker.Value.WorkerLoadBalancing != nullptr)
					{
						WriteLoadbalancingSection(Writer, Worker.Key, *Worker.Value.WorkerLoadBalancing, Worker.Value.bManualWorkerConnectionOnly);
					}
				}
				Writer->WriteArrayEnd();
			Writer->WriteObjectEnd(); // Load balancing section end
			Writer->WriteArrayStart(TEXT("workers")); // Workers section begin
				for (const auto& Worker : InWorkers)
				{
					if (Worker.Value.WorkerLoadBalancing != nullptr)
					{
						WriteWorkerSection(Writer, Worker.Key, Worker.Value);
					}
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

bool ValidateGeneratedLaunchConfig(const FSpatialLaunchConfigDescription& LaunchConfigDesc, const TMap<FName, FWorkerTypeLaunchSection>& InWorkers)
{
	const USpatialGDKSettings* SpatialGDKRuntimeSettings = GetDefault<USpatialGDKSettings>();

	if (!ensure(InWorkers.Num() == SpatialGDKRuntimeSettings->ServerWorkerTypes.Num()))
	{
		return false;
	}

	for (const FName& WorkerType : SpatialGDKRuntimeSettings->ServerWorkerTypes)
	{
		if(!ensure(InWorkers.Contains(WorkerType)))
		{
			return false;
		}
	}

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

	if (!InWorkers.Contains(SpatialGDKRuntimeSettings->DefaultWorkerType.WorkerTypeName))
	{
		const EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(TEXT("Default Worker Type is invalid, please choose a valid worker type as the default.\n\nDo you want to configure your project settings now?")));

		if (Result == EAppReturnType::Yes)
		{
			FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer("Project", "SpatialGDKEditor", "Runtime Settings");
		}

		return false;
	}

	if (SpatialGDKRuntimeSettings->bEnableOffloading)
	{
		for (const TPair<FName, FActorGroupInfo>& ActorGroup : SpatialGDKRuntimeSettings->ActorGroups)
		{
			if (!SpatialGDKRuntimeSettings->ServerWorkerTypes.Contains(ActorGroup.Value.OwningWorkerType.WorkerTypeName))
			{
				const EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(FString::Printf(TEXT("Actor Group '%s' has an invalid Owning Worker Type, please choose a valid worker type.\n\nDo you want to configure your project settings now?"), *ActorGroup.Key.ToString())));

				if (Result == EAppReturnType::Yes)
				{
					FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer("Project", "SpatialGDKEditor", "Runtime Settings");
				}

				return false;
			}
		}
	}

	return true;
}

#undef LOCTEXT_NAMESPACE
