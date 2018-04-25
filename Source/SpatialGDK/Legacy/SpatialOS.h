// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "EngineMinimal.h"
#include "ScopedViewCallbacks.h"
#include "SpatialGDKViewTypes.h"
#include "SpatialGDKWorkerConfiguration.h"
#include "SpatialGDKWorkerConfigurationData.h"
#include "SpatialGDKWorkerTypes.h"
#include "WorkerConnection.h"

#include "SpatialOS.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialOS, Log, All);

class UCallbackDispatcher;
class UEntityPipeline;

// clang-format off
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConnectedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDisconnectedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConnectionFailedDelegate);
// clang-format on

/**
 * This class provides an interface for customising a worker, connecting and disconnecting from
 * SpatialOS.
 */
UCLASS()
class SPATIALGDK_API USpatialOS : public UObject
{
	GENERATED_BODY()
  public:
	USpatialOS();

	virtual void BeginDestroy() override;

	/**
   * Applies customizations to the worker configuration.
   *
   * Remarks:
   * It is invalid to call this after this.Connect is called.
   */
	// clang-format off
  UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	// clang-format on
	void ApplyConfiguration(const FSpatialGDKWorkerConfigurationData& InWorkerConfigurationData);

	/**
  * Applies a worker configuration specified in the editor to a PIE worker.
  * If multiple worker instances are started, worker configurations are applied to the first worker
  * instances based on the "PIEInstance" field of their respective "WorldContext".
  * If there are more worker instances than worker configurations, no worker configuration is
  * applied to the remaining worker instances.
  *
  * Remarks:
  * Only call this method for worker instances started from the Unity Editor.
  * Calling this method yields no effect if InWorldContext.WorldType != EWorldType::PIE or
  * InWorldContext.PIEInstance == -1.
  * Do not call this method if any of the following editor settings assumes a certain value:
  * * "Run Dedicated Server" is true.
  * * "Auto Connect To Server" is true.
  * * "Use Single Process" is false.
  * It is invalid to call this method after this.Connect is called.
  */
	void ApplyEditorWorkerConfiguration(FWorldContext& InWorldContext);

	/**
   * Start the bootstrap process which results in connecting to SpatialOS.
   *
   * Remarks:
   * It is invalid to call this method before ApplyConfiguration has been called.
   * It is invalid to call this multiple times between matching calls to Disconnect.
   */
	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	void Connect();

	/**
  * Call this to cleanly disconnect from SpatialOS.
  */
	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	void Disconnect();

	/**
   * True if a valid connection exists to SpatialOS.
   */
	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	bool IsConnected() const;

	/**
   * Processes ops from the SpatialOS connection and triggers callbacks.
   * This should be called periodically, typically every frame.
   *
   * Remarks:
   * If a connection attempt has failed, call this method to receive log messages and disconnection
   * events.
   */
	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	void ProcessOps();

	/**
   * Provides access to the configured worker.
   *
   * Remarks:
   * ApplyConfiguration must be called before access to this property is valid.
   */
	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	const FSpatialGDKWorkerConfiguration GetWorkerConfiguration() const;

	/**
  * Returns the ID that was assigned to this worker at runtime.
  * Remarks:
  * Your worker must be connected before before access to this property is valid.
  */
	UFUNCTION(BlueprintPure, Category = "SpatialOS")
	const FString GetWorkerId() const;

	/**
   * Returns the attributes associated with this worker at runtime.
   * Remarks:
   * Your worker must be connected before before access to this property is valid.
   */
	UFUNCTION(BlueprintPure, Category = "SpatialOS")
	const TArray<FString> GetWorkerAttributes() const;

	/**
   * Provides callbacks for events related to the connection and updates.
   */
	TWeakPtr<SpatialOSView> GetView();

	/**
   * The connection to SpatialOS.
   */
	TWeakPtr<SpatialOSConnection> GetConnection();

	/**
   * The connection to SpatialOS.
   */
	const TWeakPtr<SpatialOSConnection> GetConnection() const;

	/**
   * The current metrics.
   */
	worker::Metrics& GetMetrics();

	/**
  * Entity pipeline
  */
	UEntityPipeline* GetEntityPipeline() const;

	/**
  * Callback dispatcher
  */
	UCallbackDispatcher* GetCallbackDispatcher() const;

	/**
   * Returns the raw entity, if it is available on this worker, or nullptr otherwise.
   */
	worker::Entity* GetLocalEntity(const worker::EntityId& InEntityId);

	/**
   * This delegate is triggered when a connection to SpatialOS has been successfully made.
   */
	UPROPERTY(BlueprintAssignable, Category = "SpatialOS")
	FOnConnectedDelegate OnConnectedDelegate;

	/**
   * This delegate is triggered when the connection to SpatialOS is terminated for any reason.
   */
	UPROPERTY(BlueprintAssignable, Category = "SpatialOS")
	FOnDisconnectedDelegate OnDisconnectedDelegate;

	/**
   * This delegate is triggered when a connection to SpatialOS has not been successfully made.
   */
	UPROPERTY(BlueprintAssignable, Category = "SpatialOS")
	FOnConnectionFailedDelegate OnConnectionFailedDelegate;

  private:
	UPROPERTY()
	FSpatialGDKWorkerConfiguration WorkerConfiguration;

	UPROPERTY()
	UEntityPipeline* EntityPipeline;

	UPROPERTY()
	UCallbackDispatcher* CallbackDispatcher;

	improbable::unreal::core::FWorkerConnection WorkerConnection;
	bool bConnectionWasSuccessful;

	improbable::unreal::callbacks::FScopedViewCallbacks Callbacks;
	void OnDisconnectDispatcherCallback(const worker::DisconnectOp& Op);
	void OnDisconnectInternal();
};