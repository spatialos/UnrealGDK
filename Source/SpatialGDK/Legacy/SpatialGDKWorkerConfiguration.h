// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKWorkerConfigurationData.h"
#include "SpatialGDKWorkerTypes.h"
#include "SpatialGDKWorkerConfiguration.generated.h"

USTRUCT(BlueprintType)
struct SPATIALGDK_API FSpatialGDKWorkerConfiguration
{
	GENERATED_USTRUCT_BODY()
  public:
	FSpatialGDKWorkerConfiguration();
	FSpatialGDKWorkerConfiguration(const FSpatialGDKWorkerConfigurationData& WorkerConfigurationData,
								   const TArray<FString>* const CommandLineOverrides = nullptr);

  DEPRECATED(12.1, "Use GetProjectName instead.")
  const FString& GetAppName() const;
  DEPRECATED(12.1, "This field is deprecated and will be removed in a future release.")
  const FString& GetAssemblyName() const;
  const FString& GetDeploymentName() const;
  const FString& GetDeploymentTag() const;
  const FString& GetProjectName() const;
  const FString& GetWorkerType() const;
  const FString& GetWorkerId() const;
  const FString& GetLoginToken() const;
  const FString& GetLocatorHost() const;
  const FString& GetReceptionistHost() const;
  const worker::NetworkConnectionType GetLinkProtocol() const;
  const uint32 GetReceptionistPort() const;
  const uint32 GetRaknetHeartbeatTimeoutMillis() const;
  const uint32 GetReceiveQueueCapacity() const;
  const uint32 GetSendQueueCapacity() const;
  const FString& GetSteamToken() const;
  const uint32 GetTcpMultiplexLevel() const;
  const bool GetTcpNoDelay() const;
  const uint32 GetTcpReceiveBufferSize() const;
  const uint32 GetTcpSendBufferSize() const;
  DEPRECATED(12.1, "This field is deprecated and will be removed in a future release.")
  const FString& GetInfraServiceUrl() const;
  const uint32 GetBuiltInMetricsReportPeriodMillis() const;
  const uint32 GetLogMessageQueueCapacity() const;
  const bool GetProtocolLoggingOnStartup() const;
  const FString& GetProtocolLogPrefix() const;
  const uint32 GetProtocolLogMaxFileBytes() const;
  const uint32 GetProtocolLogMaxFiles() const;
  DEPRECATED(12.1, "This field is deprecated and will be removed in a future release.")
  const bool GetShowDebugTraces() const;
  DEPRECATED(12.1, "This field is deprecated and will be removed in a future release.")
  const bool GetUseInstrumentation() const;
  DEPRECATED(12.1, "This field is deprecated and will be removed in a future release.")
  const bool GetLogDebugToSpatialOs() const;
  DEPRECATED(12.1, "This field is deprecated and will be removed in a future release.")
  const bool GetLogAssertToSpatialOs() const;
  DEPRECATED(12.1, "This field is deprecated and will be removed in a future release.")
  const bool GetLogWarningToSpatialOs() const;
  DEPRECATED(12.1, "This field is deprecated and will be removed in a future release.")
  const bool GetLogErrorToSpatialOs() const;
  const bool GetUseExternalIp() const;

  private:
	void LogWorkerConfiguration() const;

	FSpatialGDKWorkerConfigurationData WorkerConfigurationData;
};
