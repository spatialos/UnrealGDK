// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialOSWorkerConfiguration.h"

#include "CommandLine.h"
#include "SpatialOS.h"
#include "WorkerConnection.h"

namespace
{
FString GenerateRandomWorkerIdFromTypeName(const FString& type)
{
  return type + FGuid::NewGuid().ToString();
}
}  // ::

FSpatialOSWorkerConfiguration::FSpatialOSWorkerConfiguration() : WorkerConfigurationData()
{
}

FSpatialOSWorkerConfiguration::FSpatialOSWorkerConfiguration(
    const FSpatialOSWorkerConfigurationData& InWorkerConfigurationData,
    const TArray<FString>* const InCommandLineOverrides)
: WorkerConfigurationData(InWorkerConfigurationData)
{
  const auto commandLine = FCommandLine::Get();
  auto& AppConfig = WorkerConfigurationData.SpatialOSApplication;

  FParse::Value(commandLine, *FString("projectName"), AppConfig.ProjectName);

  if (AppConfig.ProjectName.IsEmpty())
  {
    UE_LOG(
        LogSpatialOS, Error,
        TEXT("The project's name must be set from code, or on the command line with +projectName."))
  }

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
  NetworkConfig.LinkProtocol = LinkProtocolString == "Tcp" ? worker::NetworkConnectionType::kTcp
                                                           : worker::NetworkConnectionType::kRaknet;

  FParse::Value(commandLine, *FString("receptionistPort"), NetworkConfig.ReceptionistPort);
  FParse::Value(commandLine, *FString("raknetHeartbeatTimeoutMs"),
                NetworkConfig.RaknetHeartbeatTimeoutMillis);
  FParse::Value(commandLine, *FString("receiveQueueCapacity"), NetworkConfig.ReceiveQueueCapacity);
  FParse::Value(commandLine, *FString("sendQueueCapacity"), NetworkConfig.SendQueueCapacity);
  FParse::Value(commandLine, *FString("steamToken"), NetworkConfig.SteamToken);
  FParse::Value(commandLine, *FString("tcpMultiplexLevel"), NetworkConfig.TcpMultiplexLevel);
  FParse::Bool(commandLine, *FString("tcpNoDelay"), NetworkConfig.TcpNoDelay);
  FParse::Value(commandLine, *FString("tcpReceiveBufferSize"), NetworkConfig.TcpReceiveBufferSize);
  FParse::Value(commandLine, *FString("tcpSendBufferSize"), NetworkConfig.TcpSendBufferSize);
  FParse::Bool(commandLine, *FString("useExternalIpForBridge"), NetworkConfig.UseExternalIp);

  auto& DebuggingConfig = WorkerConfigurationData.Debugging;
  FParse::Value(commandLine, *FString("builtInMetricsReportPeriodMillis"),
                DebuggingConfig.BuiltInMetricsReportPeriodMillis);
  FParse::Value(commandLine, *FString("logMessageQueueCapacity"),
                DebuggingConfig.LogMessageQueueCapacity);
  FParse::Bool(commandLine, *FString("protocolLoggingOnStartup"),
               DebuggingConfig.ProtocolLoggingOnStartup);
  FParse::Value(commandLine, *FString("protocolLogPrefix"), DebuggingConfig.ProtocolLogPrefix);
  FParse::Value(commandLine, *FString("protocolLogMaxFileBytes"),
                DebuggingConfig.ProtocolLogMaxFileBytes);
  FParse::Value(commandLine, *FString("protocolLogMaxFiles"), DebuggingConfig.ProtocolLogMaxFiles);

  LogWorkerConfiguration();
}

const FString& FSpatialOSWorkerConfiguration::GetDeploymentName() const
{
  return WorkerConfigurationData.SpatialOSApplication.DeploymentName;
}

const FString& FSpatialOSWorkerConfiguration::GetDeploymentTag() const
{
  return WorkerConfigurationData.SpatialOSApplication.DeploymentTag;
}

const FString& FSpatialOSWorkerConfiguration::GetProjectName() const
{
  return WorkerConfigurationData.SpatialOSApplication.ProjectName;
}

const FString& FSpatialOSWorkerConfiguration::GetWorkerType() const
{
  return WorkerConfigurationData.SpatialOSApplication.WorkerPlatform;
}

const FString& FSpatialOSWorkerConfiguration::GetWorkerId() const
{
  return WorkerConfigurationData.SpatialOSApplication.WorkerId;
}

const FString& FSpatialOSWorkerConfiguration::GetLoginToken() const
{
  return WorkerConfigurationData.SpatialOSApplication.LoginToken;
}

const FString& FSpatialOSWorkerConfiguration::GetLocatorHost() const
{
  return WorkerConfigurationData.Networking.LocatorHost;
}

const FString& FSpatialOSWorkerConfiguration::GetReceptionistHost() const
{
  return WorkerConfigurationData.Networking.ReceptionistHost;
}

const worker::NetworkConnectionType FSpatialOSWorkerConfiguration::GetLinkProtocol() const
{
  return WorkerConfigurationData.Networking.LinkProtocol;
}

const uint32 FSpatialOSWorkerConfiguration::GetReceptionistPort() const
{
  return WorkerConfigurationData.Networking.ReceptionistPort;
}

const uint32 FSpatialOSWorkerConfiguration::GetRaknetHeartbeatTimeoutMillis() const
{
  return WorkerConfigurationData.Networking.RaknetHeartbeatTimeoutMillis;
}

const uint32 FSpatialOSWorkerConfiguration::GetReceiveQueueCapacity() const
{
  return WorkerConfigurationData.Networking.ReceiveQueueCapacity;
}

const uint32 FSpatialOSWorkerConfiguration::GetSendQueueCapacity() const
{
  return WorkerConfigurationData.Networking.SendQueueCapacity;
}

const FString& FSpatialOSWorkerConfiguration::GetSteamToken() const
{
  return WorkerConfigurationData.Networking.SteamToken;
}

const uint32 FSpatialOSWorkerConfiguration::GetTcpMultiplexLevel() const
{
  return WorkerConfigurationData.Networking.TcpMultiplexLevel;
}

const bool FSpatialOSWorkerConfiguration::GetTcpNoDelay() const
{
  return WorkerConfigurationData.Networking.TcpNoDelay;
}

const uint32 FSpatialOSWorkerConfiguration::GetTcpReceiveBufferSize() const
{
  return WorkerConfigurationData.Networking.TcpReceiveBufferSize;
}

const uint32 FSpatialOSWorkerConfiguration::GetTcpSendBufferSize() const
{
  return WorkerConfigurationData.Networking.TcpSendBufferSize;
}

const uint32 FSpatialOSWorkerConfiguration::GetBuiltInMetricsReportPeriodMillis() const
{
  return WorkerConfigurationData.Debugging.BuiltInMetricsReportPeriodMillis;
}

const uint32 FSpatialOSWorkerConfiguration::GetLogMessageQueueCapacity() const
{
  return WorkerConfigurationData.Debugging.LogMessageQueueCapacity;
}

const bool FSpatialOSWorkerConfiguration::GetProtocolLoggingOnStartup() const
{
  return WorkerConfigurationData.Debugging.ProtocolLoggingOnStartup;
}

const FString& FSpatialOSWorkerConfiguration::GetProtocolLogPrefix() const
{
  return WorkerConfigurationData.Debugging.ProtocolLogPrefix;
}

const uint32 FSpatialOSWorkerConfiguration::GetProtocolLogMaxFileBytes() const
{
  return WorkerConfigurationData.Debugging.ProtocolLogMaxFileBytes;
}

const uint32 FSpatialOSWorkerConfiguration::GetProtocolLogMaxFiles() const
{
  return WorkerConfigurationData.Debugging.ProtocolLogMaxFiles;
}

const bool FSpatialOSWorkerConfiguration::GetUseExternalIp() const
{
  return WorkerConfigurationData.Networking.UseExternalIp;
}

void FSpatialOSWorkerConfiguration::LogWorkerConfiguration() const
{
  UE_LOG(LogSpatialOS, Display, TEXT("WorkerConfiguration settings"));
  const auto& AppConfig = WorkerConfigurationData.SpatialOSApplication;
  UE_LOG(LogSpatialOS, Display, TEXT("deploymentName = %s"), *AppConfig.DeploymentName);
  UE_LOG(LogSpatialOS, Display, TEXT("deploymentTag = %s"), *AppConfig.DeploymentTag);
  UE_LOG(LogSpatialOS, Display, TEXT("projectName = %s"), *AppConfig.ProjectName);
  UE_LOG(LogSpatialOS, Display, TEXT("workerType = %s"), *AppConfig.WorkerPlatform);
  UE_LOG(LogSpatialOS, Display, TEXT("workerId = %s"), *AppConfig.WorkerId);
  UE_LOG(LogSpatialOS, Display, TEXT("loginToken = %s"), *AppConfig.LoginToken);

  const auto& NetworkConfig = WorkerConfigurationData.Networking;
  UE_LOG(LogSpatialOS, Display, TEXT("locatorHost = %s"), *NetworkConfig.LocatorHost);
  UE_LOG(LogSpatialOS, Display, TEXT("receptionistHost = %s"), *NetworkConfig.ReceptionistHost);
  UE_LOG(LogSpatialOS, Display, TEXT("linkProtocol = %s"),
         NetworkConfig.LinkProtocol == worker::NetworkConnectionType::kTcp ? TEXT("Tcp")
                                                                           : TEXT("Raknet"));
  UE_LOG(LogSpatialOS, Display, TEXT("receptionistPort = %d"), NetworkConfig.ReceptionistPort);
  UE_LOG(LogSpatialOS, Display, TEXT("raknetHeartbeatTimeoutMs = %d"),
         NetworkConfig.RaknetHeartbeatTimeoutMillis);
  UE_LOG(LogSpatialOS, Display, TEXT("receiveQueueCapacity = %d"),
         NetworkConfig.ReceiveQueueCapacity);
  UE_LOG(LogSpatialOS, Display, TEXT("sendQueueCapacity = %d"), NetworkConfig.SendQueueCapacity);
  UE_LOG(LogSpatialOS, Display, TEXT("steamToken = %s"), *NetworkConfig.SteamToken);
  UE_LOG(LogSpatialOS, Display, TEXT("tcpMultiplexLevel = %d"), NetworkConfig.TcpMultiplexLevel);
  UE_LOG(LogSpatialOS, Display, TEXT("tcpNoDelay = %s"),
         NetworkConfig.TcpNoDelay ? TEXT("true") : TEXT("false"));
  UE_LOG(LogSpatialOS, Display, TEXT("tcpReceiveBufferSize = %d"),
         NetworkConfig.TcpReceiveBufferSize);
  UE_LOG(LogSpatialOS, Display, TEXT("tcpSendBufferSize = %d"), NetworkConfig.TcpSendBufferSize);
  UE_LOG(LogSpatialOS, Display, TEXT("UseExternalIp = %s"),
         NetworkConfig.UseExternalIp ? TEXT("true") : TEXT("false"));

  const auto& DebuggingConfig = WorkerConfigurationData.Debugging;
  UE_LOG(LogSpatialOS, Display, TEXT("builtInMetricsReportPeriodMillis = %d"),
         DebuggingConfig.BuiltInMetricsReportPeriodMillis);
  UE_LOG(LogSpatialOS, Display, TEXT("logMessageQueueCapacity = %d"),
         DebuggingConfig.LogMessageQueueCapacity);
  UE_LOG(LogSpatialOS, Display, TEXT("protocolLoggingOnStartup = %s"),
         DebuggingConfig.ProtocolLoggingOnStartup ? TEXT("true") : TEXT("false"));
  UE_LOG(LogSpatialOS, Display, TEXT("protocolLogPrefix = %s"), *DebuggingConfig.ProtocolLogPrefix);
  UE_LOG(LogSpatialOS, Display, TEXT("protocolLogMaxFileBytes = %d"),
         DebuggingConfig.ProtocolLogMaxFileBytes);
  UE_LOG(LogSpatialOS, Display, TEXT("protocolLogMaxFiles = %d"),
         DebuggingConfig.ProtocolLogMaxFiles);
}
