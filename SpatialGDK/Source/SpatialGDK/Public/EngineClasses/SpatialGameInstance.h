// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Utils/SpatialActorGroupManager.h"

#include "SpatialGameInstance.generated.h"

class USpatialLatencyTracer;
class USpatialConnectionManager;
class UGlobalStateManager;
class USpatialStaticComponentView;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGameInstance, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConnectedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectionFailedEvent, const FString&, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerSpawnFailedEvent, const FString&, Reason);

UCLASS(config = Engine)
class SPATIALGDK_API USpatialGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual FGameInstancePIEResult StartPlayInEditorGameInstance(ULocalPlayer* LocalPlayer, const FGameInstancePIEParameters& Params) override;
#endif
	// Initializes the Spatial connection if Spatial networking is enabled, otherwise does nothing.
	void TryConnectToSpatial();

	virtual void StartGameInstance() override;

	//~ Begin UObject Interface
	virtual bool ProcessConsoleExec(const TCHAR* Cmd, FOutputDevice& Ar, UObject* Executor) override;
	//~ End UObject Interface

	//~ Begin UGameInstance Interface
	virtual void Init() override;
	//~ End UGameInstance Interface

	// The SpatiaConnectionManager must always be owned by the SpatialGameInstance and so must be created here to prevent TrimMemory from deleting it during Browse.
	void CreateNewSpatialConnectionManager();

	// Destroying the SpatialConnectionManager disconnects us from SpatialOS.
	void DestroySpatialConnectionManager();

	FORCEINLINE USpatialConnectionManager* GetSpatialConnectionManager() { return SpatialConnectionManager; }
	FORCEINLINE USpatialLatencyTracer* GetSpatialLatencyTracer() { return SpatialLatencyTracer; }
	FORCEINLINE UGlobalStateManager* GetGlobalStateManager() { return GlobalStateManager; };
	FORCEINLINE USpatialStaticComponentView* GetStaticComponentView() { return StaticComponentView; };

	void HandleOnConnected();
	void HandleOnConnectionFailed(const FString& Reason);
	void HandleOnPlayerSpawnFailed(const FString& Reason);

	// Invoked when this worker has successfully connected to SpatialOS
	UPROPERTY(BlueprintAssignable)
	FOnConnectedEvent OnSpatialConnected;
	// Invoked when this worker fails to initiate a connection to SpatialOS
	UPROPERTY(BlueprintAssignable)
	FOnConnectionFailedEvent OnSpatialConnectionFailed;
	// Invoked when the player could not be spawned
	UPROPERTY(BlueprintAssignable)
	FOnPlayerSpawnFailedEvent OnSpatialPlayerSpawnFailed;

	void SetFirstConnectionToSpatialOSAttempted() { bFirstConnectionToSpatialOSAttempted = true; };
	bool GetFirstConnectionToSpatialOSAttempted() const { return bFirstConnectionToSpatialOSAttempted; };

	TUniquePtr<SpatialActorGroupManager> ActorGroupManager;

protected:
	// Checks whether the current net driver is a USpatialNetDriver.
	// Can be used to decide whether to use Unreal networking or SpatialOS networking.
	bool HasSpatialNetDriver() const;

private:
	// SpatialConnection is stored here for persistence between map travels.
	UPROPERTY()
	USpatialConnectionManager* SpatialConnectionManager;

	bool bFirstConnectionToSpatialOSAttempted = false;

	UPROPERTY()
	USpatialLatencyTracer* SpatialLatencyTracer = nullptr;

	// GlobalStateManager must persist when server traveling
	UPROPERTY()
	UGlobalStateManager* GlobalStateManager;

	// StaticComponentView must persist when server traveling
	UPROPERTY()
	USpatialStaticComponentView* StaticComponentView;

	UFUNCTION()
	void OnLevelInitializedNetworkActors(ULevel* Level, UWorld* OwningWorld);
};
