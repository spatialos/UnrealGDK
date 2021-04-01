// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKSettings.h"

#include "Improbable/SpatialEngineConstants.h"
#include "Misc/CommandLine.h"
#include "Misc/MessageDialog.h"

#include "SpatialConstants.h"
#include "Utils/GDKPropertyMacros.h"
#include "Utils/SpatialStatics.h"

#if WITH_EDITOR
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Settings/LevelEditorPlaySettings.h"

#include "SpatialGDKServicesConstants.h"
#include "SpatialGDKServicesModule.h"
#endif

DEFINE_LOG_CATEGORY(LogSpatialGDKSettings);

#define LOCTEXT_NAMESPACE "SpatialGDKSettings"

namespace
{
constexpr int32 DefaultEventTracingFileSize = 256 * 1024 * 1024; // 256mb

void CheckCmdLineOverrideBool(const TCHAR* CommandLine, const TCHAR* Parameter, const TCHAR* PrettyName, bool& bOutValue)
{
#if ALLOW_SPATIAL_CMDLINE_PARSING // Command-line only enabled for non-shipping or with target rule bEnableSpatialCmdlineInShipping enabled
	if (FParse::Param(CommandLine, Parameter))
	{
		bOutValue = true;
	}
	else
	{
		TCHAR TempStr[16];
		if (FParse::Value(CommandLine, Parameter, TempStr, 16) && TempStr[0] == '=')
		{
			bOutValue = FCString::ToBool(TempStr + 1); // + 1 to skip =
		}
	}
#endif // ALLOW_SPATIAL_CMDLINE_PARSING
	UE_LOG(LogSpatialGDKSettings, Log, TEXT("%s is %s."), PrettyName, bOutValue ? TEXT("enabled") : TEXT("disabled"));
}

void CheckCmdLineOverrideOptionalBool(const TCHAR* CommandLine, const TCHAR* Parameter, const TCHAR* PrettyName, TOptional<bool>& bOutValue)
{
#if ALLOW_SPATIAL_CMDLINE_PARSING // Command-line only enabled for non-shipping or with target rule bEnableSpatialCmdlineInShipping enabled
	if (FParse::Param(CommandLine, Parameter))
	{
		bOutValue = true;
	}
	else
	{
		TCHAR TempStr[16];
		if (FParse::Value(CommandLine, Parameter, TempStr, 16) && TempStr[0] == '=')
		{
			bOutValue = FCString::ToBool(TempStr + 1); // + 1 to skip =
		}
	}
#endif // ALLOW_SPATIAL_CMDLINE_PARSING
	UE_LOG(LogSpatialGDKSettings, Log, TEXT("%s is %s."), PrettyName,
		   bOutValue.IsSet() ? bOutValue ? TEXT("enabled") : TEXT("disabled") : TEXT("not set"));
}

void CheckCmdLineOverrideOptionalString(const TCHAR* CommandLine, const TCHAR* Parameter, const TCHAR* PrettyName,
										TOptional<FString>& StrOutValue)
{
#if ALLOW_SPATIAL_CMDLINE_PARSING
	FString TempStr;
	if (FParse::Value(CommandLine, Parameter, TempStr) && TempStr[0] == '=')
	{
		StrOutValue = TempStr.Right(TempStr.Len() - 1); // + 1 to skip =
	}
#endif // ALLOW_SPATIAL_CMDLINE_PARSING
	UE_LOG(LogSpatialGDKSettings, Log, TEXT("%s is %s."), PrettyName, StrOutValue.IsSet() ? *(StrOutValue.GetValue()) : TEXT("not set"));
}

void CheckCmdLineOverrideOptionalStringWithCallback(const TCHAR* CommandLine, const TCHAR* Parameter, const TCHAR* PrettyName,
													TFunctionRef<void(const FString& Setting)> Callback)
{
#if ALLOW_SPATIAL_CMDLINE_PARSING
	FString TempStr;
	TOptional<FString> OverrideValue;
	if (FParse::Value(CommandLine, Parameter, TempStr) && TempStr[0] == '=')
	{
		OverrideValue = TempStr.Right(TempStr.Len() - 1); // + 1 to skip =
		Callback(OverrideValue.GetValue());
	}
#endif // ALLOW_SPATIAL_CMDLINE_PARSING
	UE_LOG(LogSpatialGDKSettings, Log, TEXT("%s is %s."), PrettyName,
		   OverrideValue.IsSet() ? *(OverrideValue.GetValue()) : TEXT("not set"));
}
} // namespace

USpatialGDKSettings::USpatialGDKSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, EntityPoolInitialReservationCount(3000)
	, EntityPoolRefreshThreshold(1000)
	, EntityPoolRefreshCount(2000)
	, HeartbeatIntervalSeconds(2.0f)
	, HeartbeatTimeoutSeconds(10.0f)
	, HeartbeatTimeoutWithEditorSeconds(10000.0f)
	, ActorReplicationRateLimit(0)
	, EntityCreationRateLimit(0)
	, bUseIsActorRelevantForConnection(false)
	, OpsUpdateRate(1000.0f)
	, MaxNetCullDistanceSquared(0.0f) // Default disabled
	, QueuedIncomingRPCWaitTime(1.0f)
	, QueuedIncomingRPCRetryTime(1.0f)
	, QueuedOutgoingRPCRetryTime(1.0f)
	, PositionUpdateLowerThresholdSeconds(1.0f)		  // 1 second
	, PositionUpdateLowerThresholdCentimeters(100.0f) // 1m (100cm)
	, PositionUpdateThresholdMaxSeconds(60.0f)		  // 1 minute (60 seconds)
	, PositionUpdateThresholdMaxCentimeters(5000.0f)  // 50m (5000cm)
	, bEnableMetrics(true)
	, bEnableMetricsDisplay(false)
	, MetricsReportRate(2.0f)
	, bUseFrameTimeAsLoad(false)
	, bBatchSpatialPositionUpdates(false)
	, MaxDynamicallyAttachedSubobjectsPerClass(3)
	, ServicesRegion(EServicesRegion::Default)
	, WorkerLogLevel(ESettingsWorkerLogVerbosity::Warning) // Deprecated - UNR-4348
	, LocalWorkerLogLevel(WorkerLogLevel)
	, CloudWorkerLogLevel(WorkerLogLevel)
	, bEnableMultiWorker(true)
	, DefaultRPCRingBufferSize(32)
	, CrossServerRPCImplementation(ECrossServerRPCImplementation::SpatialCommand)
	// TODO - UNR 2514 - These defaults are not necessarily optimal - readdress when we have better data
	, bTcpNoDelay(false)
	, UdpServerDownstreamUpdateIntervalMS(1)
	, UdpClientDownstreamUpdateIntervalMS(1)
	, bWorkerFlushAfterOutgoingNetworkOp(true)
	// TODO - end
	, bAsyncLoadNewClassesOnEntityCheckout(false)
	, RPCQueueWarningDefaultTimeout(2.0f)
	, bEnableNetCullDistanceInterest(true)
	, bEnableNetCullDistanceFrequency(false)
	, FullFrequencyNetCullDistanceRatio(1.0f)
	, bUseSecureClientConnection(false)
	, bUseSecureServerConnection(false)
	, bEnableClientQueriesOnServer(false)
	, bEnableCrossLayerActorSpawning(true)
	, StartupLogRate(5.0f)
	, ActorMigrationLogRate(5.0f)
	, bEventTracingEnabled(false)
	, EventTracingSamplingSettingsClass(UEventTracingSamplingSettings::StaticClass())
	, MaxEventTracingFileSizeBytes(DefaultEventTracingFileSize)
	, bEnableAlwaysWriteRPCs(false)
	, bEnableInitialOnlyReplicationCondition(false)
{
	DefaultReceptionistHost = SpatialConstants::LOCAL_HOST;
	RPCRingBufferSizeOverrides.Add(ERPCType::ServerAlwaysWrite, 1);
}

void USpatialGDKSettings::PostInitProperties()
{
	Super::PostInitProperties();

	// Check any command line overrides for using QBI, Offloading (after reading the config value):
	const TCHAR* CommandLine = FCommandLine::Get();
	CheckCmdLineOverrideBool(CommandLine, TEXT("EnableCrossLayerActorSpawning"), TEXT("Multiserver cross-layer Actor spawning"),
							 bEnableCrossLayerActorSpawning);
	CheckCmdLineOverrideBool(CommandLine, TEXT("OverrideNetCullDistanceInterest"), TEXT("Net cull distance interest"),
							 bEnableNetCullDistanceInterest);
	CheckCmdLineOverrideBool(CommandLine, TEXT("OverrideNetCullDistanceInterestFrequency"), TEXT("Net cull distance interest frequency"),
							 bEnableNetCullDistanceFrequency);
	CheckCmdLineOverrideBool(CommandLine, TEXT("OverrideActorRelevantForConnection"), TEXT("Actor relevant for connection"),
							 bUseIsActorRelevantForConnection);
	CheckCmdLineOverrideBool(CommandLine, TEXT("OverrideBatchSpatialPositionUpdates"), TEXT("Batch spatial position updates"),
							 bBatchSpatialPositionUpdates);
	CheckCmdLineOverrideBool(CommandLine, TEXT("OverridePreventClientCloudDeploymentAutoConnect"),
							 TEXT("Prevent client cloud deployment auto connect"), bPreventClientCloudDeploymentAutoConnect);
	CheckCmdLineOverrideBool(CommandLine, TEXT("OverrideWorkerFlushAfterOutgoingNetworkOp"),
							 TEXT("Flush worker ops after sending an outgoing network op."), bWorkerFlushAfterOutgoingNetworkOp);
	CheckCmdLineOverrideBool(CommandLine, TEXT("OverrideEventTracingEnabled"), TEXT("Event tracing enabled"), bEventTracingEnabled);
	CheckCmdLineOverrideOptionalString(CommandLine, TEXT("OverrideMultiWorkerSettingsClass"), TEXT("Override MultiWorker Settings Class"),
									   OverrideMultiWorkerSettingsClass);
	CheckCmdLineOverrideOptionalStringWithCallback(
		CommandLine, TEXT("OverrideEventTracingSamplingSettingsClass"), TEXT("Override Event Tracing Sampling Class"),
		[&SamplingSettingsClass = EventTracingSamplingSettingsClass](const FString& OverrideValue) {
			FSoftClassPath OverrideSampleSettingsSoftClassPath(OverrideValue);
			UClass* OverrideSampleSettingsClass = OverrideSampleSettingsSoftClassPath.TryLoadClass<UEventTracingSamplingSettings>();
			if (OverrideSampleSettingsClass != nullptr)
			{
				SamplingSettingsClass = OverrideSampleSettingsClass;
			}
			else
			{
				UE_LOG(LogSpatialGDKSettings, Log, TEXT("Invalid event tracing sampling class specified: %s."), *OverrideValue);
			}
		});
	UE_LOG(LogSpatialGDKSettings, Log, TEXT("Spatial Networking is %s."),
		   USpatialStatics::IsSpatialNetworkingEnabled() ? TEXT("enabled") : TEXT("disabled"));
}

#if WITH_EDITOR
void USpatialGDKSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Use MemberProperty here so we report the correct member name for nested changes
	const FName Name = (PropertyChangedEvent.MemberProperty != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	// TODO(UNR-3569): Engine PR to remove bEnableOffloading from ULevelEditorPlaySettings.

	if (Name == GET_MEMBER_NAME_CHECKED(USpatialGDKSettings, MaxDynamicallyAttachedSubobjectsPerClass))
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			LOCTEXT("RegenerateSchemaDynamicSubobjects_Prompt",
					"You MUST regenerate schema using the full scan option after changing the number of max dynamic subobjects. "
					"Failing to do will result in unintended behavior or crashes!"));
	}
	else if (Name == GET_MEMBER_NAME_CHECKED(USpatialGDKSettings, ServicesRegion))
	{
		UpdateServicesRegionFile();
	}
}

bool USpatialGDKSettings::CanEditChange(const GDK_PROPERTY(Property) * InProperty) const
{
	if (!InProperty)
	{
		return false;
	}

	const FName Name = InProperty->GetFName();

	return true;
}

void USpatialGDKSettings::UpdateServicesRegionFile()
{
	// Create or remove an empty file in the plugin directory indicating whether to use China services region.
	const FString UseChinaServicesRegionFilepath =
		FSpatialGDKServicesModule::GetSpatialGDKPluginDirectory(SpatialGDKServicesConstants::UseChinaServicesRegionFilename);
	if (IsRunningInChina())
	{
		if (!FPaths::FileExists(UseChinaServicesRegionFilepath))
		{
			FFileHelper::SaveStringToFile(TEXT(""), *UseChinaServicesRegionFilepath);
		}
	}
	else
	{
		if (FPaths::FileExists(UseChinaServicesRegionFilepath))
		{
			FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*UseChinaServicesRegionFilepath);
		}
	}
}

#endif // WITH_EDITOR

uint32 USpatialGDKSettings::GetRPCRingBufferSize(ERPCType RPCType) const
{
	if (const uint32* Size = RPCRingBufferSizeOverrides.Find(RPCType))
	{
		return *Size;
	}

	return DefaultRPCRingBufferSize;
}

float USpatialGDKSettings::GetSecondsBeforeWarning(const ERPCResult Result) const
{
	if (const float* CustomSecondsBeforeWarning = RPCQueueWarningTimeouts.Find(Result))
	{
		return *CustomSecondsBeforeWarning;
	}

	return RPCQueueWarningDefaultTimeout;
}

bool USpatialGDKSettings::ShouldRPCTypeAllowUnresolvedParameters(const ERPCType Type) const
{
	if (const bool* LogSetting = RPCTypeAllowUnresolvedParamMap.Find(Type))
	{
		return *LogSetting;
	}

	return false;
}

void USpatialGDKSettings::SetServicesRegion(EServicesRegion::Type NewRegion)
{
	ServicesRegion = NewRegion;

	// Save in default config so this applies for other platforms e.g. Linux, Android.
	GDK_PROPERTY(Property)* ServicesRegionProperty = USpatialGDKSettings::StaticClass()->FindPropertyByName(FName("ServicesRegion"));
	UpdateSinglePropertyInConfigFile(ServicesRegionProperty, GetDefaultConfigFilename());
}

bool USpatialGDKSettings::GetPreventClientCloudDeploymentAutoConnect() const
{
	return (IsRunningGame() || IsRunningClientOnly()) && bPreventClientCloudDeploymentAutoConnect;
};

UEventTracingSamplingSettings* USpatialGDKSettings::GetEventTracingSamplingSettings() const
{
	return EventTracingSamplingSettingsClass != nullptr
			   ? EventTracingSamplingSettingsClass->GetDefaultObject<UEventTracingSamplingSettings>()
			   : UEventTracingSamplingSettings::StaticClass()->GetDefaultObject<UEventTracingSamplingSettings>();
}

#if WITH_EDITOR
void USpatialGDKSettings::SetMultiWorkerEditorEnabled(bool bIsEnabled)
{
	bEnableMultiWorker = bIsEnabled;
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE
