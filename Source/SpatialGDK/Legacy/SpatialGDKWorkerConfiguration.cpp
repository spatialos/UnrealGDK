// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialGDKWorkerConfiguration.h"

#include "CommandLine.h"
#include "SpatialOS.h"
#include "WorkerConnection.h"

namespace
{
FString GenerateRandomWorkerIdFromTypeName(const FString& type)
{
	return type + FGuid::NewGuid().ToString();
}
} // ::

FSpatialGDKWorkerConfiguration::FSpatialGDKWorkerConfiguration()
	: WorkerConfigurationData()
{
}

FSpatialGDKWorkerConfiguration::FSpatialGDKWorkerConfiguration(
	const FSpatialGDKWorkerConfigurationData& InWorkerConfigurationData,
	const TArray<FString>* const InCommandLineOverrides)
	: WorkerConfigurationData(InWorkerConfigurationData)
{
	const auto commandLine = FCommandLine::Get();
	auto& AppConfig = WorkerConfigurationData.SpatialGDKApplication;

	FParse::Value(commandLine, *FString("appName"), AppConfig.AppName);
	FParse::Value(commandLine, *FString("projectName"), AppConfig.ProjectName);
	if (!AppConfig.AppName.IsEmpty() && AppConfig.ProjectName.IsEmpty())
	{
		AppConfig.ProjectName = AppConfig.AppName;
	}

	if (AppConfig.ProjectName.IsEmpty())
	{
		UE_LOG(LogSpatialOS, Error, TEXT("The project's name must be set from code, or on the "
										 "command line with +projectName."))
	}

	FParse::Value(commandLine, *FString("assemblyName"), AppConfig.AssemblyName);

	FParse::Value(commandLine, *FString("deploymentName"), AppConfig.DeploymentName);
	const auto SplitIndex = AppConfig.DeploymentName.Find("-");
	AppConfig.DeploymentName.RemoveAt(0, SplitIndex + 1);

	FParse::Value(commandLine, *FString("deploymentTag"), AppConfig.DeploymentTag);
	FParse::Value(commandLine, *FString("workerType"), AppConfig.WorkerPlatform);
	FParse::Value(commandLine, *FString("workerId"), AppConfig.WorkerId);

	if (AppConfig.WorkerId.IsEmpty())
	{
		AppConfig.WorkerId = GenerateRandomWorkerIdFromTypeName(AppConfig.WorkerPlatform);
	}

	FParse::Value(commandLine, *FString("loginToken"), AppConfig.LoginToken);

	auto& NetworkConfig = WorkerConfigurationData.Networking;
	FParse::Value(commandLine, *FString("locatorHost"), NetworkConfig.LocatorHost);
	FParse::Value(commandLine, *FString("receptionistHost"), NetworkConfig.ReceptionistHost);

	auto LinkProtocolString = FString("");
	FParse::Value(commandLine, *FString("linkProtocol"), LinkProtocolString);
	NetworkConfig.LinkProtocol = LinkProtocolString == "Tcp"
									 ? worker::NetworkConnectionType::kTcp
									 : worker::NetworkConnectionType::kRaknet;

	FParse::Value(commandLine, *FString("receptionistPort"), NetworkConfig.ReceptionistPort);
	FParse::Value(commandLine, *FString("raknetHeartbeatTimeoutMs"), NetworkConfig.RaknetHeartbeatTimeoutMillis);
	FParse::Value(commandLine, *FString("receiveQueueCapacity"), NetworkConfig.ReceiveQueueCapacity);
	FParse::Value(commandLine, *FString("sendQueueCapacity"), NetworkConfig.SendQueueCapacity);
	FParse::Value(commandLine, *FString("steamToken"), NetworkConfig.SteamToken);
	FParse::Value(commandLine, *FString("tcpMultiplexLevel"), NetworkConfig.TcpMultiplexLevel);
	FParse::Bool(commandLine, *FString("tcpNoDelay"), NetworkConfig.TcpNoDelay);
	FParse::Value(commandLine, *FString("tcpReceiveBufferSize"), NetworkConfig.TcpReceiveBufferSize);
	FParse::Value(commandLine, *FString("tcpSendBufferSize"), NetworkConfig.TcpSendBufferSize);
	FParse::Bool(commandLine, *FString("useExternalIpForBridge"), NetworkConfig.UseExternalIp);

	auto& DebuggingConfig = WorkerConfigurationData.Debugging;
	FParse::Value(commandLine, *FString("infraServicesUrl"), DebuggingConfig.InfraServiceUrl);
	FParse::Value(commandLine, *FString("builtInMetricsReportPeriodMillis"), DebuggingConfig.BuiltInMetricsReportPeriodMillis);
	FParse::Value(commandLine, *FString("logMessageQueueCapacity"), DebuggingConfig.LogMessageQueueCapacity);
	FParse::Bool(commandLine, *FString("protocolLoggingOnStartup"), DebuggingConfig.ProtocolLoggingOnStartup);
	FParse::Value(commandLine, *FString("protocolLogPrefix"), DebuggingConfig.ProtocolLogPrefix);
	FParse::Value(commandLine, *FString("protocolLogMaxFileBytes"), DebuggingConfig.ProtocolLogMaxFileBytes);
	FParse::Value(commandLine, *FString("protocolLogMaxFiles"), DebuggingConfig.ProtocolLogMaxFiles);
	FParse::Bool(commandLine, *FString("showDebugTraces"), DebuggingConfig.ShowDebugTraces);
	FParse::Bool(commandLine, *FString("useInstrumentation"), DebuggingConfig.UseInstrumentation);

	FParse::Bool(commandLine, *FString("logDebugToSpatialOs"), DebuggingConfig.LogDebugToSpatialOs);
	FParse::Bool(commandLine, *FString("logAssertToSpatialOs"), DebuggingConfig.LogAssertToSpatialOs);
	FParse::Bool(commandLine, *FString("logWarningToSpatialOs"), DebuggingConfig.LogWarningToSpatialOs);
	FParse::Bool(commandLine, *FString("logErrorToSpatialOs"), DebuggingConfig.LogErrorToSpatialOs);

	LogWorkerConfiguration();
}

const FString& FSpatialGDKWorkerConfiguration::GetAppName() const
{
	return WorkerConfigurationData.SpatialGDKApplication.AppName;
}

const FString& FSpatialGDKWorkerConfiguration::GetAssemblyName() const
{
	return WorkerConfigurationData.SpatialGDKApplication.AssemblyName;
}

const FString& FSpatialGDKWorkerConfiguration::GetDeploymentName() const
{
	return WorkerConfigurationData.SpatialGDKApplication.DeploymentName;
}

const FString& FSpatialGDKWorkerConfiguration::GetDeploymentTag() const
{
	return WorkerConfigurationData.SpatialGDKApplication.DeploymentTag;
}

const FString& FSpatialGDKWorkerConfiguration::GetProjectName() const
{
	return WorkerConfigurationData.SpatialGDKApplication.ProjectName;
}

const FString& FSpatialGDKWorkerConfiguration::GetWorkerType() const
{
	return WorkerConfigurationData.SpatialGDKApplication.WorkerPlatform;
}

const FString& FSpatialGDKWorkerConfiguration::GetWorkerId() const
{
	return WorkerConfigurationData.SpatialGDKApplication.WorkerId;
}

const FString& FSpatialGDKWorkerConfiguration::GetLoginToken() const
{
	return WorkerConfigurationData.SpatialGDKApplication.LoginToken;
}

const FString& FSpatialGDKWorkerConfiguration::GetLocatorHost() const
{
	return WorkerConfigurationData.Networking.LocatorHost;
}

const FString& FSpatialGDKWorkerConfiguration::GetReceptionistHost() const
{
	return WorkerConfigurationData.Networking.ReceptionistHost;
}

const worker::NetworkConnectionType FSpatialGDKWorkerConfiguration::GetLinkProtocol() const
{
	return WorkerConfigurationData.Networking.LinkProtocol;
}

const uint32 FSpatialGDKWorkerConfiguration::GetReceptionistPort() const
{
	return WorkerConfigurationData.Networking.ReceptionistPort;
}

const uint32 FSpatialGDKWorkerConfiguration::GetRaknetHeartbeatTimeoutMillis() const
{
	return WorkerConfigurationData.Networking.RaknetHeartbeatTimeoutMillis;
}

const uint32 FSpatialGDKWorkerConfiguration::GetReceiveQueueCapacity() const
{
	return WorkerConfigurationData.Networking.ReceiveQueueCapacity;
}

const uint32 FSpatialGDKWorkerConfiguration::GetSendQueueCapacity() const
{
	return WorkerConfigurationData.Networking.SendQueueCapacity;
}

const FString& FSpatialGDKWorkerConfiguration::GetSteamToken() const
{
	return WorkerConfigurationData.Networking.SteamToken;
}

const uint32 FSpatialGDKWorkerConfiguration::GetTcpMultiplexLevel() const
{
	return WorkerConfigurationData.Networking.TcpMultiplexLevel;
}

const bool FSpatialGDKWorkerConfiguration::GetTcpNoDelay() const
{
	return WorkerConfigurationData.Networking.TcpNoDelay;
}

const uint32 FSpatialGDKWorkerConfiguration::GetTcpReceiveBufferSize() const
{
	return WorkerConfigurationData.Networking.TcpReceiveBufferSize;
}

const uint32 FSpatialGDKWorkerConfiguration::GetTcpSendBufferSize() const
{
	return WorkerConfigurationData.Networking.TcpSendBufferSize;
}

const FString& FSpatialGDKWorkerConfiguration::GetInfraServiceUrl() const
{
	return WorkerConfigurationData.Debugging.InfraServiceUrl;
}

const uint32 FSpatialGDKWorkerConfiguration::GetBuiltInMetricsReportPeriodMillis() const
{
	return WorkerConfigurationData.Debugging.BuiltInMetricsReportPeriodMillis;
}

const uint32 FSpatialGDKWorkerConfiguration::GetLogMessageQueueCapacity() const
{
	return WorkerConfigurationData.Debugging.LogMessageQueueCapacity;
}

const bool FSpatialGDKWorkerConfiguration::GetProtocolLoggingOnStartup() const
{
	return WorkerConfigurationData.Debugging.ProtocolLoggingOnStartup;
}

const FString& FSpatialGDKWorkerConfiguration::GetProtocolLogPrefix() const
{
	return WorkerConfigurationData.Debugging.ProtocolLogPrefix;
}

const uint32 FSpatialGDKWorkerConfiguration::GetProtocolLogMaxFileBytes() const
{
	return WorkerConfigurationData.Debugging.ProtocolLogMaxFileBytes;
}

const uint32 FSpatialGDKWorkerConfiguration::GetProtocolLogMaxFiles() const
{
	return WorkerConfigurationData.Debugging.ProtocolLogMaxFiles;
}

const bool FSpatialGDKWorkerConfiguration::GetShowDebugTraces() const
{
	return WorkerConfigurationData.Debugging.ShowDebugTraces;
}

const bool FSpatialGDKWorkerConfiguration::GetUseInstrumentation() const
{
	return WorkerConfigurationData.Debugging.UseInstrumentation;
}

const bool FSpatialGDKWorkerConfiguration::GetLogDebugToSpatialOs() const
{
	return WorkerConfigurationData.Debugging.LogDebugToSpatialOs;
}

const bool FSpatialGDKWorkerConfiguration::GetLogAssertToSpatialOs() const
{
	return WorkerConfigurationData.Debugging.LogAssertToSpatialOs;
}

const bool FSpatialGDKWorkerConfiguration::GetLogWarningToSpatialOs() const
{
	return WorkerConfigurationData.Debugging.LogWarningToSpatialOs;
}

const bool FSpatialGDKWorkerConfiguration::GetLogErrorToSpatialOs() const
{
	return WorkerConfigurationData.Debugging.LogErrorToSpatialOs;
}

const bool FSpatialGDKWorkerConfiguration::GetUseExternalIp() const
{
	return WorkerConfigurationData.Networking.UseExternalIp;
}

void FSpatialGDKWorkerConfiguration::LogWorkerConfiguration() const
{
	UE_LOG(LogSpatialOS, Display, TEXT("WorkerConfiguration settings"));
	const auto& AppConfig = WorkerConfigurationData.SpatialGDKApplication;
	UE_LOG(LogSpatialOS, Display, TEXT("assemblyName = %s"), *AppConfig.AssemblyName);
	UE_LOG(LogSpatialOS, Display, TEXT("deploymentName = %s"), *AppConfig.DeploymentName);
	UE_LOG(LogSpatialOS, Display, TEXT("deploymentTag = %s"), *AppConfig.DeploymentTag);
	UE_LOG(LogSpatialOS, Display, TEXT("projectName = %s"), *AppConfig.ProjectName);
	UE_LOG(LogSpatialOS, Display, TEXT("workerType = %s"), *AppConfig.WorkerPlatform);
	UE_LOG(LogSpatialOS, Display, TEXT("workerId = %s"), *AppConfig.WorkerId);
	UE_LOG(LogSpatialOS, Display, TEXT("loginToken = %s"), *AppConfig.LoginToken);

	const auto& NetworkConfig = WorkerConfigurationData.Networking;
	UE_LOG(LogSpatialOS, Display, TEXT("locatorHost = %s"), *NetworkConfig.LocatorHost);
	UE_LOG(LogSpatialOS, Display, TEXT("receptionistHost = %s"), *NetworkConfig.ReceptionistHost);
	UE_LOG(LogSpatialOS, Display, TEXT("linkProtocol = %s"), NetworkConfig.LinkProtocol == worker::NetworkConnectionType::kTcp ? TEXT("Tcp") : TEXT("Raknet"));
	UE_LOG(LogSpatialOS, Display, TEXT("receptionistPort = %d"), NetworkConfig.ReceptionistPort);
	UE_LOG(LogSpatialOS, Display, TEXT("raknetHeartbeatTimeoutMs = %d"), NetworkConfig.RaknetHeartbeatTimeoutMillis);
	UE_LOG(LogSpatialOS, Display, TEXT("receiveQueueCapacity = %d"), NetworkConfig.ReceiveQueueCapacity);
	UE_LOG(LogSpatialOS, Display, TEXT("sendQueueCapacity = %d"), NetworkConfig.SendQueueCapacity);
	UE_LOG(LogSpatialOS, Display, TEXT("steamToken = %s"), *NetworkConfig.SteamToken);
	UE_LOG(LogSpatialOS, Display, TEXT("tcpMultiplexLevel = %d"), NetworkConfig.TcpMultiplexLevel);
	UE_LOG(LogSpatialOS, Display, TEXT("tcpNoDelay = %s"), NetworkConfig.TcpNoDelay ? TEXT("true") : TEXT("false"));
	UE_LOG(LogSpatialOS, Display, TEXT("tcpReceiveBufferSize = %d"), NetworkConfig.TcpReceiveBufferSize);
	UE_LOG(LogSpatialOS, Display, TEXT("tcpSendBufferSize = %d"), NetworkConfig.TcpSendBufferSize);
	UE_LOG(LogSpatialOS, Display, TEXT("UseExternalIp = %s"), NetworkConfig.UseExternalIp ? TEXT("true") : TEXT("false"));

	const auto& DebuggingConfig = WorkerConfigurationData.Debugging;
	UE_LOG(LogSpatialOS, Display, TEXT("infraServicesUrl = %s"), *DebuggingConfig.InfraServiceUrl);
	UE_LOG(LogSpatialOS, Display, TEXT("logDebugToSpatialOs = %s"), DebuggingConfig.LogDebugToSpatialOs ? TEXT("true") : TEXT("false"));
	UE_LOG(LogSpatialOS, Display, TEXT("logAssertToSpatialOs = %s"), DebuggingConfig.LogAssertToSpatialOs ? TEXT("true") : TEXT("false"));
	UE_LOG(LogSpatialOS, Display, TEXT("logWarningToSpatialOs = %s"), DebuggingConfig.LogWarningToSpatialOs ? TEXT("true") : TEXT("false"));
	UE_LOG(LogSpatialOS, Display, TEXT("logErrorToSpatialOs = %s"), DebuggingConfig.LogErrorToSpatialOs ? TEXT("true") : TEXT("false"));
	UE_LOG(LogSpatialOS, Display, TEXT("builtInMetricsReportPeriodMillis = %d"), DebuggingConfig.BuiltInMetricsReportPeriodMillis);
	UE_LOG(LogSpatialOS, Display, TEXT("logMessageQueueCapacity = %d"), DebuggingConfig.LogMessageQueueCapacity);
	UE_LOG(LogSpatialOS, Display, TEXT("protocolLoggingOnStartup = %s"), DebuggingConfig.ProtocolLoggingOnStartup ? TEXT("true") : TEXT("false"));
	UE_LOG(LogSpatialOS, Display, TEXT("protocolLogPrefix = %s"), *DebuggingConfig.ProtocolLogPrefix);
	UE_LOG(LogSpatialOS, Display, TEXT("protocolLogMaxFileBytes = %d"), DebuggingConfig.ProtocolLogMaxFileBytes);
	UE_LOG(LogSpatialOS, Display, TEXT("protocolLogMaxFiles = %d"), DebuggingConfig.ProtocolLogMaxFiles);
	UE_LOG(LogSpatialOS, Display, TEXT("showDebugTraces = %s"), DebuggingConfig.ShowDebugTraces ? TEXT("true") : TEXT("false"));
	UE_LOG(LogSpatialOS, Display, TEXT("useInstrumentation = %s"), DebuggingConfig.UseInstrumentation ? TEXT("true") : TEXT("false"));
}
