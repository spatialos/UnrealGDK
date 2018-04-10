// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "HAL/ThreadSafeBool.h"
#include "SpatialGDKLoader.h"
#include "SpatialOSViewTypes.h"
#include "SpatialOSWorkerConfiguration.h"

namespace improbable
{
namespace unreal
{
namespace core
{
DECLARE_DELEGATE_RetVal_OneParam(bool, FQueueStatusDelegate, const worker::QueueStatus&)
    DECLARE_DELEGATE_OneParam(FOnConnectedDelegate, bool)
        DECLARE_DELEGATE_OneParam(FOnDeploymentsFoundDelegate, worker::DeploymentList)

    /**
    * Manages the lifecycle of a connection to SpatialOS.
    * Periodically reports metrics to the SpatialOS.
    *
    * It is not safe to use any member of this class in different threads.
    */
    class FWorkerConnection
{
public:
  /** Returns true if there is an active connection to SpatialOS. */
  bool IsConnected() const;

  /**
  * Constructs a new worker connection.
  * The Components list allows the newly-constructed connection to serialize and deserialize
  * components that exist in the game's module.
  */
  FWorkerConnection();

  /** Create a worker::LocatorParameters struct with values populated from a
   * FSpatialOSWorkerConfiguration struct. */
  worker::LocatorParameters
  CreateLocatorParameters(const FSpatialOSWorkerConfiguration& WorkerConfiguration);

  /** Get a list of available deployemnts to connect to. This is non blocking and will call
   * OnDeplymentsFoundCallback when deployments are found, or the timout is reached */
  void GetDeploymentListAsync(const FString& LocatorHost,
                              const worker::LocatorParameters& LocatorParams,
                              FOnDeploymentsFoundDelegate OnDeploymentsFoundCallback,
                              std::uint32_t TimeoutMillis = DefaultTimeout);

  /** Connect to SpatialOS. This is non blocking and will call OnConnectedCallback when either the
   * creation is made or the timeout is reached */
  void ConnectToReceptionistAsync(const FString& Hostname, uint16 Port, const FString& WorkerId,
                                  const worker::ConnectionParameters& Params,
                                  FOnConnectedDelegate OnConnectedCallback,
                                  std::uint32_t TimeoutMillis = DefaultTimeout);

  /** Connect to SpatialOS. This is non blocking and will call OnConnectedCallback when either the
   * creation is made or the timeout is reached */
  void ConnectToLocatorAsync(const FString& LocatorHost,
                             const worker::LocatorParameters& LocatorParams,
                             const FString& DeploymentId,
                             const worker::ConnectionParameters& Params,
                             FQueueStatusDelegate QueueStatusCallback,
                             FOnConnectedDelegate OnConnectedCallback,
                             std::uint32_t TimeoutMillis = DefaultTimeout);

  /** Terminates an existing connection to SpatialOS. */
  void Disconnect();
  /** Process any ops that have been dispatched from the SpatialOS connection. */
  void ProcessOps();
  /** Send a message to SpatialOS. */
  void SendLogMessage(ELogVerbosity::Type Level, const FString& Message);

  TWeakPtr<SpatialOSView> GetView();

  const TWeakPtr<SpatialOSView> GetView() const;

  TWeakPtr<SpatialOSConnection> GetConnection();

  const TWeakPtr<SpatialOSConnection> GetConnection() const;

  /** Returns the metrics object that is reported to SpatialOS. */
  worker::Metrics& GetMetrics();

private:
  DECLARE_DELEGATE(FMetricsDelegate);

  bool CanCreateNewConnection() const;

  void WaitForDeploymentFuture(std::uint32_t TimeoutMillis,
                               SpatialOSFuture<worker::DeploymentList> DeploymentListFuture,
                               FOnDeploymentsFoundDelegate OnDeploymentsFoundCallback);

  void WaitForConnectionFuture(std::uint32_t TimeoutMillis, SpatialOSFuture<SpatialOSConnection>,
                               FOnConnectedDelegate OnConnectedCallback);

  void OnMetrics(const worker::MetricsOp& Op);

  FSpatialGDKLoader SdkLoader;

  FThreadSafeBool bIsConnecting;

  static constexpr std::uint32_t DefaultTimeout = 50000u;

  TSharedPtr<SpatialOSView> View;
  TSharedPtr<SpatialOSConnection> Connection;

  FTimerHandle MetricsReporterHandle;
  TUniquePtr<worker::Metrics> Metrics;
  FMetricsDelegate MetricsDelegate;
};
}  // ::core
}  // ::unreal
}  // ::improbable
