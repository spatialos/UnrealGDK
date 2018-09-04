// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include <improbable/worker.h>
#include "improbable/defaults.h"

#include "SpatialGDKWorkerConfigurationData.generated.h"

/**
* Provides configuration parameters related to how a worker establishes a
* connection with a
* SpatialOS deployment.
*/
USTRUCT(BlueprintType)
struct SPATIALGDK_API FSpatialGDKNetworkingConfiguration
{
	GENERATED_USTRUCT_BODY()

	FSpatialGDKNetworkingConfiguration()
	: LocatorHost("locator.improbable.io")
	, ReceptionistHost("127.0.0.1")
	, LinkProtocol(worker::NetworkConnectionType::kRaknet)
	, ReceptionistPort(7777)
	, RaknetHeartbeatTimeoutMillis(worker::defaults::kRakNetHeartbeatTimeoutMillis)
	, ReceiveQueueCapacity(worker::defaults::kReceiveQueueCapacity)
	, SendQueueCapacity(worker::defaults::kSendQueueCapacity)
	, SteamToken("")
	, TcpMultiplexLevel(worker::defaults::kTcpMultiplexLevel)
	, TcpNoDelay(worker::defaults::kTcpNoDelay)
	, TcpReceiveBufferSize(worker::defaults::kTcpReceiveBufferSize)
	, TcpSendBufferSize(worker::defaults::kTcpSendBufferSize)
	, UseExternalIp(worker::defaults::kUseExternalIp)
	{
	}

	/**
  * "hostname" argument for a constructor call of an underlying worker::Locator
  * object.
  * Target hostname for a locator.
  * To be used when connecting to a SpatialOS deployment via a locator service.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration)
	FString LocatorHost;
	/**
  * "hostname" argument for an underlying worker::Connection::ConnectAsync()
  * function call.
  * Target hostname for connecting to a deployment via receptionist.
  * This is the flow used to connect a managed worker running in the cloud
  * alongside the
  * deployment,
  * and also to connect any local worker to a (local or remote) deployment via a
  * locally-running
  * receptionist.
  * To be used when connecting to a SpatialOS deployment via a receptionist.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration)
	FString ReceptionistHost;
	/**
  * "ConnectionType" worker::NetworkConnectionType enum field for an underlying
  * worker::NetworkParameters struct.
  * Type of network connection to use when connecting to SpatialOS.
  * Used when establishing a connection with a SpatialOS deployment.
  */
	worker::NetworkConnectionType LinkProtocol;
	/**
  * "port" argument for an underlying worker::Connection::ConnectAsync()
  * function call.
  * Target port for connecting to a deployment via receptionist.
  * This is the flow used to connect a managed worker running in the cloud
  * alongside the
  * deployment,
  * and also to connect any local worker to a (local or remote) deployment via a
  * locally-running
  * receptionist.
  * To be used when connecting to a SpatialOS deployment via a receptionist.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration)
	uint32 ReceptionistPort;
	/**
  * "HeartbeatTimeoutMillis" field for an underlying
  * worker::RakNetNetworkParameters struct.
  * Time (in milliseconds) that RakNet should use for its heartbeat protocol.
  * To be used when RakNet is chosen as the underlying network connection
  * type/link protocol.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration)
	uint32 RaknetHeartbeatTimeoutMillis;
	/**
  * "ReceiveQueueCapacity" field for an underlying worker::ConnectionParameters
  * struct.
  * Number of messages that can be stored on the receive queue.
  * Used when establishing a connection with a SpatialOS deployment.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration)
	uint32 ReceiveQueueCapacity;
	/**
  * "SendQueueCapacity" field for an underlying worker::ConnectionParameters
  * struct.
  * Number of messages that can be stored on the send queue.
  * Used when establishing a connection with a SpatialOS deployment.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration)
	uint32 SendQueueCapacity;
	/**
  * "Ticket" field for an underlying worker::SteamCredentials struct.
  * Steam ticket for the steam app ID and publisher key corresponding to the
  * project name
  * specified
  * in the LocatorParameters. Typically obtained from the steam APIs.
  * To be used when connecting to a SpatialOS deployment from Steam.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration)
	FString SteamToken;
	/**
  * "MultiplexLevel" field for an underlying worker::TcpNetworkParameters
  * struct.
  * Number of multiplexed TCP connections.
  * To be used when TCP is chosen as the underlying network connection type/link
  * protocol.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration)
	uint32 TcpMultiplexLevel;
	/**
  * "NoDelay" field for an underlying worker::TcpNetworkParameters struct.
  * Whether to enable TCP_NODELAY.
  * To be used when TCP is chosen as the underlying network connection type/link
  * protocol.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration)
	bool TcpNoDelay;
	/**
  * "ReceiveBufferSize" field for an underlying worker::TcpNetworkParameters
  * struct.
  * Size in bytes of the TCP receive buffer.
  * To be used when TCP is chosen as the underlying network connection type/link
  * protocol.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration)
	uint32 TcpReceiveBufferSize;
	/**
  * "SendBufferSize" field for an underlying worker::TcpNetworkParameters
  * struct.
  * Size in bytes of the TCP send buffer.
  * To be used when TCP is chosen as the underlying network connection type/link
  * protocol.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration)
	uint32 TcpSendBufferSize;
	/**
  * "UseExternalIp" field for an underlying worker::NetworkParameters struct.
  * Set this flag to connect to SpatialOS using the externally-visible IP
  * address.
  * Used when establishing a connection with a SpatialOS deployment.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration)
	bool UseExternalIp;
};

/**
* Provides configuration parameters related to debugging a worker.
*/
USTRUCT(BlueprintType)
struct SPATIALGDK_API FSpatialGDKDebuggingConfig
{
	GENERATED_USTRUCT_BODY()

	FSpatialGDKDebuggingConfig()
	: InfraServiceUrl("https://api.spatial.improbable.io")
	, LogDebugToSpatialOs(false)
	, LogAssertToSpatialOs(false)
	, LogWarningToSpatialOs(true)
	, LogErrorToSpatialOs(true)
	, BuiltInMetricsReportPeriodMillis(worker::defaults::kBuiltInMetricsReportPeriodMillis)
	, LogMessageQueueCapacity(worker::defaults::kLogMessageQueueCapacity)
	, ProtocolLoggingOnStartup(false)
	, ProtocolLogPrefix(worker::defaults::kLogPrefix.c_str())
	, ProtocolLogMaxFileBytes(100U * worker::defaults::kMaxLogFileSizeBytes)
	, ProtocolLogMaxFiles(worker::defaults::kMaxLogFiles)
	, ShowDebugTraces(false)
	, UseInstrumentation(true)
	{
	}

	/**
  * This field is not in use. Setting this field will not affect a worker's
  * behaviour.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration, meta = (DeprecatedFunction, DeprecationMessage = "This field is deprecated and will be removed in a future release."))
	FString InfraServiceUrl;
	/**
  * This field is not in use. Setting this field will not affect a worker's
  * behaviour.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration, meta = (DeprecatedFunction, DeprecationMessage = "This field is deprecated and will be removed in a future release."))
	bool LogDebugToSpatialOs;
	/**
  * This field is not in use. Setting this field will not affect a worker's
  * behaviour.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration, meta = (DeprecatedFunction, DeprecationMessage = "This field is deprecated and will be removed in a future release."))
	bool LogAssertToSpatialOs;
	/**
  * This field is not in use. Setting this field will not affect a worker's
  * behaviour.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration, meta = (DeprecatedFunction, DeprecationMessage = "This field is deprecated and will be removed in a future release."))
	bool LogWarningToSpatialOs;
	/**
  * This field is not in use. Setting this field will not affect a worker's
  * behaviour.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration, meta = (DeprecatedFunction, DeprecationMessage = "This field is deprecated and will be removed in a future release."))
	bool LogErrorToSpatialOs;
	/**
  * "BuiltInMetricsReportPeriodMillis" field for an underlying
  * worker::ConnectionParameters
  * struct.
  * This parameter controls how frequently the Connection will return a
  * MetricsOp reporting its
  * built-in metrics. If set to zero, this functionality is disabled.
  * Used when establishing a connection with a SpatialOS deployment.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration)
	uint32 BuiltInMetricsReportPeriodMillis;
	/**
  * "LogMessageQueueCapacity" field for an underlying
  * worker::ConnectionParameters struct.
  * Number of messages logged by the SDK that can be stored in the log message
  * queue.
  * Used when establishing a connection with a SpatialOS deployment.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration)
	uint32 LogMessageQueueCapacity;
	/**
  * "EnableProtocolLoggingAtStartpup" field for an underlying
  * worker::ConnectionParameters struct.
  * Whether to enable protocol logging at startup.
  * Used when establishing a connection with a SpatialOS deployment.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration)
	bool ProtocolLoggingOnStartup;
	/**
  * "LogPrefix" field for an underlying worker::ProtocolLoggingParameters
  * struct.
  * Log file names are prefixed with this prefix, are numbered, and have the
  * extension .log.
  * To be used when protocol logging is enabled.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration)
	FString ProtocolLogPrefix;
	/**
  * "MaxLogFileSizeBytes" field for an underlying
  * worker::ProtocolLoggingParameters struct.
  * Once the size of a log file reaches this size, a new log file is created.
  * To be used when protocol logging is enabled.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration)
	uint32 ProtocolLogMaxFileBytes;
	/**
  * "MaxLogFiles" field for an underlying worker::ProtocolLoggingParameters
  * struct.
  * Maximum number of log files to keep.
  * To be used when protocol logging is enabled.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration)
	uint32 ProtocolLogMaxFiles;
	/**
  * This field is not in use. Setting this field will not affect a worker's
  * behaviour.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration, meta = (DeprecatedFunction, DeprecationMessage = "This field is deprecated and will be removed in a future release."))
	bool ShowDebugTraces;
	/**
  * This field is not in use. Setting this field will not affect a worker's
  * behaviour.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration, meta = (DeprecatedFunction, DeprecationMessage = "This field is deprecated and will be removed in a future release."))
	bool UseInstrumentation;
};

/**
* Provides configuration parameters related to a worker's behaviour.
*/
USTRUCT(BlueprintType)
struct SPATIALGDK_API FSpatialGDKApplicationConfig
{
	GENERATED_USTRUCT_BODY()

	FSpatialGDKApplicationConfig() : AppName(""), AssemblyName(""), DeploymentName(""), DeploymentTag("prod"), ProjectName(""), WorkerPlatform("UnrealClient"), WorkerId(""), LoginToken("")
	{
	}

	/**
  * "ProjectName" field for an underlying worker::LocatorParameters struct.
  * To be used when connecting to a SpatialOS deployment via a locator
  * service.
  * This field is deprecated. Please use ProjectName instead.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration, meta = (DeprecatedFunction, DeprecationMessage = "This field is deprecated, use ProjectName instead."))
	FString AppName;
	/**
  * This field is not in use. Setting this field will not affect a worker's
  * behaviour.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration, meta = (DeprecatedFunction, DeprecationMessage = "This field is deprecated and will be removed in a future release."))
	FString AssemblyName;
	/**
  * "deployment_name" argument for an underlying
  * worker::Locator::ConnectAsync() function call.
  * The name of the deployment.
  * To be used when connecting to a SpatialOS deployment via a locator
  * service.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration)
	FString DeploymentName;
	/**
  * "DeploymentTag" field for an underlying worker::SteamCredentials struct.
  * Deployment tag to request access for. If non-empty, must match the following
  * regex:
  * [A-Za-z0-9][A-Za-z0-9_].
  * To be used when connecting to a SpatialOS deployment from Steam.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration)
	FString DeploymentTag;
	/**
  * "ProjectName" field for an underlying worker::LocatorParameters struct.
  * The name of the SpatialOS project.
  * To be used when connecting to a SpatialOS deployment via a locator service.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration)
	FString ProjectName;
	/**
  * "WorkerType" field for an underlying worker::ConnectionParameters struct.
  * A string that specifies the type of a worker.
  * Used when establishing a connection with a SpatialOS deployment.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration)
	FString WorkerPlatform;
	/**
  * "worker_id" argument for an underlying worker::Connection::ConnectAsync()
  * function call.
  * Self assigned worker id of the connecting worker.
  * To be used when connecting to a SpatialOS deployment via a receptionist.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration)
	FString WorkerId;
	/**
  * "LoginToken" field for an underlying worker::LocatorParameters struct.
  * Authentication token allowing a worker to connect to a SpatialOS deployment.
  * To be used when connecting to a SpatialOS deployment via a locator service.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration)
	FString LoginToken;
};

/**
* Provides configuration parameters for controlling a worker's behaviour.
*/
USTRUCT(BlueprintType)
struct SPATIALGDK_API FSpatialGDKWorkerConfigurationData
{
	GENERATED_USTRUCT_BODY()

	FSpatialGDKWorkerConfigurationData() : Debugging(), Networking(), SpatialGDKApplication()
	{
	}

	/**
  * Provides configuration parameters related to debugging a worker.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration)
	FSpatialGDKDebuggingConfig Debugging;

	/**
  * Provides configuration parameters related to how a worker establishes a
  * connection with a
  * SpatialOS deployment.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration)
	FSpatialGDKNetworkingConfiguration Networking;

	/**
  * Provides configuration parameters related to a worker's behaviour.
  */
	UPROPERTY(EditAnywhere, config, Category = WorkerConfiguration)
	FSpatialGDKApplicationConfig SpatialGDKApplication;
};
