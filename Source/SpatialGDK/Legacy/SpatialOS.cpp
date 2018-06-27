// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialOS.h"

#include "CallbackDispatcher.h"
#include "EntityPipeline.h"
#include "SpatialGDKSettings.h"
#include "SpatialGDKViewTypes.h"
#include "SpatialGDKWorkerTypes.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY(LogSpatialOS);

USpatialOS::USpatialOS()
: WorkerConfiguration(), WorkerConnection(), bConnectionWasSuccessful(false)
{
  EntityPipeline = CreateDefaultSubobject<UEntityPipeline>(TEXT("EntityPipeline"));
  CallbackDispatcher = CreateDefaultSubobject<UCallbackDispatcher>(TEXT("CallbackDispatcher"));
}

void USpatialOS::BeginDestroy()
{
  Super::BeginDestroy();
  OnDisconnectInternal();
}

void USpatialOS::ApplyConfiguration(const FSpatialGDKWorkerConfigurationData& InWorkerConfigurationData)
{
  checkf(!IsConnected(), TEXT("ApplyConfiguration was called after Connect was called."));
	WorkerConfiguration = FSpatialGDKWorkerConfiguration(InWorkerConfigurationData);
}

void USpatialOS::ApplyEditorWorkerConfiguration(FWorldContext& InWorldContext)
{
  checkf(!IsConnected(), TEXT("ApplyConfiguration was called after Connect was called."));

  // This WorldContext does not represent a PIE instance
  if (InWorldContext.WorldType != EWorldType::PIE || InWorldContext.PIEInstance == -1)
  {
    UE_LOG(LogSpatialOS, Warning,
           TEXT("USpatialOS::ApplyEditorWorkerConfiguration(): The supplied "
                "WorldContext does not represent a PIE instance. No changes are made. "
                "Make sure you only call this method when starting a worker instance "
                "from the Unreal Editor. PIEInstance: %d, WorldType: %d"),
           InWorldContext.PIEInstance, static_cast<int>(InWorldContext.WorldType));
    return;
  }

  const auto SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
  if (SpatialGDKSettings == nullptr || !SpatialGDKSettings->bUseUserWorkerConfigurations)
  {
    return;
  }

  // InWorldContext.PIEInstance is 0 if one PIE instance is run otherwise [1, ..., n] if n PIE
  // instances are run. See Engine\Source\Editor\UnrealEd\Private\PlayLevel.cpp, line 2528.
  const int32 EditorConfigurationArrayIndex = FMath::Max(0, InWorldContext.PIEInstance - 1);
  const int32 NumWorkerConfigurations = SpatialGDKSettings->WorkerConfigurations.Num();
  if (EditorConfigurationArrayIndex < NumWorkerConfigurations)
  {
    const auto& WorkerConfig =
        SpatialGDKSettings->WorkerConfigurations[EditorConfigurationArrayIndex];
    WorkerConfiguration = FSpatialGDKWorkerConfiguration(WorkerConfig.WorkerConfigurationData);

    // This check is required When a PIE instance is launched as a dedicated server,
    // as no GameViewport will have been created.
    if (InWorldContext.GameViewport != nullptr)
    {
      InWorldContext.GameViewport->bDisableWorldRendering = WorkerConfig.bDisableRendering;
    }
  }
}

bool USpatialOS::IsConnected() const
{
  return WorkerConnection.IsConnected();
}

void USpatialOS::Connect()
{
  checkf(!IsConnected(), TEXT("Connection is already established."));
  // Log parsed input
  UE_LOG(LogSpatialOS, Warning,
         TEXT(": receptionistHost %s, receptionistPort %d, WorkerType %s, WorkerId %s"),
         *WorkerConfiguration.GetReceptionistHost(), WorkerConfiguration.GetReceptionistPort(),
         *WorkerConfiguration.GetWorkerType(), *WorkerConfiguration.GetWorkerId())

  auto LockedView = WorkerConnection.GetView().Pin();
  if (LockedView.IsValid())
  {
    Callbacks.Add(LockedView->OnDisconnect(
        std::bind(&USpatialOS::OnDisconnectDispatcherCallback, this, std::placeholders::_1)));
  }

  worker::ConnectionParameters Params;
  Params.BuiltInMetricsReportPeriodMillis =
      WorkerConfiguration.GetBuiltInMetricsReportPeriodMillis();
  Params.EnableProtocolLoggingAtStartup = WorkerConfiguration.GetProtocolLoggingOnStartup();
  Params.LogMessageQueueCapacity = WorkerConfiguration.GetLogMessageQueueCapacity();
  Params.Network.ConnectionType = WorkerConfiguration.GetLinkProtocol();
  Params.Network.RakNet.HeartbeatTimeoutMillis =
      WorkerConfiguration.GetRaknetHeartbeatTimeoutMillis();
  Params.Network.Tcp.MultiplexLevel = WorkerConfiguration.GetTcpMultiplexLevel();
  Params.Network.Tcp.NoDelay = WorkerConfiguration.GetTcpNoDelay();
  Params.Network.Tcp.ReceiveBufferSize = WorkerConfiguration.GetTcpReceiveBufferSize();
  Params.Network.Tcp.SendBufferSize = WorkerConfiguration.GetTcpSendBufferSize();
  Params.Network.UseExternalIp = WorkerConfiguration.GetUseExternalIp();
  Params.ProtocolLogging.LogPrefix = TCHAR_TO_UTF8(*WorkerConfiguration.GetProtocolLogPrefix());
  Params.ProtocolLogging.MaxLogFiles = WorkerConfiguration.GetProtocolLogMaxFiles();
  Params.ProtocolLogging.MaxLogFileSizeBytes = WorkerConfiguration.GetProtocolLogMaxFileBytes();
  Params.ReceiveQueueCapacity = WorkerConfiguration.GetReceiveQueueCapacity();
  Params.SendQueueCapacity = WorkerConfiguration.GetSendQueueCapacity();
  Params.WorkerType = TCHAR_TO_UTF8(*WorkerConfiguration.GetWorkerType());

  improbable::unreal::core::FOnConnectedDelegate OnConnected;
  OnConnected.BindLambda([this](bool HasConnected) {
    bConnectionWasSuccessful = HasConnected;
    if (HasConnected)
    {
      // Broadcast() first to allow pipeline blocks to be added
      OnConnectedDelegate.Broadcast();
      UE_LOG(LogSpatialOS, Display, TEXT("Connected to SpatialOS"));

      CallbackDispatcher->Init(GetView());
      EntityPipeline->Init(GetView(), CallbackDispatcher);
    }
  });

  improbable::unreal::core::FQueueStatusDelegate OnQueueStatus;
  OnQueueStatus.BindLambda([this](const worker::QueueStatus& Status) {
    if (Status.Error)
    {
      UE_LOG(LogSpatialOS, Error, TEXT("Error connecting to deployment: %s"),
             UTF8_TO_TCHAR(Status.Error->c_str()));
      return false;
    }
    else
    {
      UE_LOG(LogSpatialOS, Display, TEXT("Position in queue: %u"), Status.PositionInQueue);
      return true;
    }
  });

  const bool ShouldConnectViaLocator = !WorkerConfiguration.GetLoginToken().IsEmpty();
  if (ShouldConnectViaLocator)
  {
    WorkerConnection.ConnectToLocatorAsync(
        WorkerConfiguration.GetProjectName(), WorkerConfiguration.GetLocatorHost(),
        WorkerConfiguration.GetDeploymentName(), WorkerConfiguration.GetLoginToken(), Params,
        OnQueueStatus, OnConnected);
  }
  else
  {
    WorkerConnection.ConnectToReceptionistAsync(
        WorkerConfiguration.GetReceptionistHost(), WorkerConfiguration.GetReceptionistPort(),
        WorkerConfiguration.GetWorkerId(), Params, OnConnected);
  }
}

void USpatialOS::Disconnect()
{
  if (IsConnected())
  {
    WorkerConnection.Disconnect();

    // Manually broadcast OnDisconnected callbacks as Dispatcher->OnDisconnected will not be called.
    OnDisconnectedDelegate.Broadcast("SpatialOS was manually disconnected.");
  }
  OnDisconnectInternal();
}

void USpatialOS::ProcessOps()
{
  WorkerConnection.ProcessOps();
}

const FSpatialGDKWorkerConfiguration USpatialOS::GetWorkerConfiguration() const
{
  return WorkerConfiguration;
}

const FString USpatialOS::GetWorkerId() const
{
  if (IsConnected())
  {
    const auto LockedConnection = GetConnection().Pin();

    if (LockedConnection.IsValid())
    {
      return FString(LockedConnection->GetWorkerId().c_str());
    }
  }
  return FString();
}

const TArray<FString> USpatialOS::GetWorkerAttributes() const
{
  TArray<FString> Attributes;
  if (IsConnected())
  {
    const auto LockedConnection = GetConnection().Pin();

    if (LockedConnection.IsValid())
    {
      const auto WorkerAttributes = LockedConnection->GetWorkerAttributes();

      for (const auto Attribute : WorkerAttributes)
      {
        Attributes.Add(FString(Attribute.c_str()));
      }
    }
  }
  return Attributes;
}

TWeakPtr<SpatialOSView> USpatialOS::GetView()
{
  return WorkerConnection.GetView();
}

TWeakPtr<SpatialOSConnection> USpatialOS::GetConnection()
{
  return WorkerConnection.GetConnection();
}

const TWeakPtr<SpatialOSConnection> USpatialOS::GetConnection() const
{
  return WorkerConnection.GetConnection();
}

worker::Metrics& USpatialOS::GetMetrics()
{
  return WorkerConnection.GetMetrics();
}

UEntityPipeline* USpatialOS::GetEntityPipeline() const
{
  return EntityPipeline;
}

UCallbackDispatcher* USpatialOS::GetCallbackDispatcher() const
{
  return CallbackDispatcher;
}

worker::Entity* USpatialOS::GetLocalEntity(const worker::EntityId& EntityId)
{
  if (!IsConnected())
  {
    return nullptr;
  }

  auto LockedView = GetView().Pin();

  if (LockedView.IsValid())
  {
    auto& entityMap = LockedView->Entities;
    auto entityIter = entityMap.find(EntityId);

    if (entityIter != entityMap.end())
    {
      return &(entityIter->second);
    }
  }

  return nullptr;
}

void USpatialOS::OnDisconnectDispatcherCallback(const worker::DisconnectOp& Op)
{
  if (bConnectionWasSuccessful)
  {
    OnDisconnectedDelegate.Broadcast(Op.Reason.c_str());
  }
  else
  {
    OnConnectionFailedDelegate.Broadcast(Op.Reason.c_str());
  }
  OnDisconnectInternal();
}

void USpatialOS::OnDisconnectInternal()
{
  CallbackDispatcher->Reset();
  EntityPipeline->DeregisterAllCallbacks();
  Callbacks.Reset();
  bConnectionWasSuccessful = false;
}
