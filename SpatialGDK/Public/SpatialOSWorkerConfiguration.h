// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "SpatialOSWorkerConfigurationData.h"
#include "SpatialGDKWorkerTypes.h"
#include "SpatialOSWorkerConfiguration.generated.h"

USTRUCT(BlueprintType)
struct SPATIALGDK_API FSpatialOSWorkerConfiguration
{
  GENERATED_USTRUCT_BODY()
public:
  FSpatialOSWorkerConfiguration();
  FSpatialOSWorkerConfiguration(const FSpatialOSWorkerConfigurationData& WorkerConfigurationData,
                                const TArray<FString>* const CommandLineOverrides = nullptr);

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
  const uint32 GetBuiltInMetricsReportPeriodMillis() const;
  const uint32 GetLogMessageQueueCapacity() const;
  const bool GetProtocolLoggingOnStartup() const;
  const FString& GetProtocolLogPrefix() const;
  const uint32 GetProtocolLogMaxFileBytes() const;
  const uint32 GetProtocolLogMaxFiles() const;
  const bool GetUseExternalIp() const;

private:
  void LogWorkerConfiguration() const;

  FSpatialOSWorkerConfigurationData WorkerConfigurationData;
};
