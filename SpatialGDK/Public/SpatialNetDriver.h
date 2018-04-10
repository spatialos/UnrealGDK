// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "IpNetDriver.h"
#include "SpatialInteropPipelineBlock.h"
#include "SpatialInterop.h"
#include "PlayerSpawnRequestSender.h"
#include "SpatialOutputDevice.h"
#include "SpatialNetDriver.generated.h"

class UEntityPipeline;
// class UEntityRegistry;
class UCallbackDispatcher;
class USpatialOSComponentUpdater;
class USpatialOS;
class USpatialNetConnection;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialOSNetDriver, Log, All);

class FSpatialWorkerUniqueNetId : public FUniqueNetId
{
public:
	FSpatialWorkerUniqueNetId(const FString& WorkerId) : WorkerId{WorkerId} {}
	~FSpatialWorkerUniqueNetId() override = default;

	const uint8* GetBytes() const override { return reinterpret_cast<const uint8*>(*WorkerId); }
	int32 GetSize() const override { return WorkerId.Len() * sizeof(TCHAR); }
	bool IsValid() const override { return true; }
	FString ToString() const override { return WorkerId; }
	FString ToDebugString() const override { return TEXT("workerId:") + WorkerId; }

private:
	FString WorkerId;
};

UCLASS()
class SPATIALGDK_API USpatialNetDriver : public UIpNetDriver
{
	GENERATED_BODY()

public:
	virtual void PostInitProperties() override;

	// Begin UNetDriver interface.
	virtual bool InitBase(bool bInitAsClient, FNetworkNotify* InNotify, const FURL& URL, bool bReuseAddressAndPort, FString& Error) override;
	virtual int32 ServerReplicateActors(float DeltaSeconds) override;
	virtual void TickDispatch(float DeltaTime) override;
	virtual void ProcessRemoteFunction(class AActor* Actor, class UFunction* Function, void* Parameters, struct FOutParmRec* OutParms, struct FFrame* NotStack, class UObject* SubObject = NULL ) override;
	virtual void TickFlush(float DeltaTime) override;
	virtual bool IsLevelInitializedForActor(const AActor* InActor, const UNetConnection* InConnection) const override;
	// End UNetDriver interface.

	USpatialOS* GetSpatialOS() const
	{
		return SpatialOSInstance;
	}

	// Returns the "100% reliable" connection to SpatialOS.
	// On the server, it is designated to be the first client connection.
	// On the client, this function is not meaningful (as we use ServerConnection)
	USpatialNetConnection* GetSpatialOSNetConnection() const;

	UPROPERTY()
	USpatialInteropPipelineBlock* InteropPipelineBlock;

	//UEntityRegistry* GetEntityRegistry() { return EntityRegistry; }

	USpatialOS* GetSpatialOS() { return SpatialOSInstance; }
	
	// Used by USpatialSpawner (when new players join the game) and USpatialInteropPipelineBlock (when player controllers are migrated).
	USpatialNetConnection* AcceptNewPlayer(const FURL& InUrl, bool bExistingPlayer);

	USpatialInterop* GetSpatialInterop() const
	{
		return Interop;
	}

protected:
	FSpatialOSWorkerConfigurationData WorkerConfig;

	UPROPERTY()
	USpatialOS* SpatialOSInstance;

	TUniquePtr<FSpatialOutputDevice> SpatialOutputDevice;

	// UPROPERTY()
	// USpatialOSComponentUpdater* SpatialOSComponentUpdater;

	// UPROPERTY()
	// UEntityRegistry* EntityRegistry;

	// Timer manager.
	FTimerManager* TimerManager;

	// Update/RPC interop with SpatialOS.
	UPROPERTY()
	USpatialInterop* Interop;

	// Package map shared by all connections.
	UPROPERTY()
	USpatialPackageMapClient* PackageMap;

	UFUNCTION()
	void OnMapLoaded(UWorld* LoadedWorld);

	UFUNCTION()
	void OnSpatialOSConnected();

	UFUNCTION()
	void OnSpatialOSConnectFailed();

	UFUNCTION()
	void OnSpatialOSDisconnected();
		
#if WITH_SERVER_CODE
	//SpatialGDK: These functions all exist in UNetDriver, but we need to modify/simplify them in certain ways.
	// Could have marked them virtual in base class but that's a pointless source change as these functions are not meant to be called from anywhere except USpatialNetDriver::ServerReplicateActors.
	int32 ServerReplicateActors_PrepConnections(const float DeltaSeconds);
	int32 ServerReplicateActors_PrioritizeActors(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers, const TArray<FNetworkObjectInfo*> ConsiderList, const bool bCPUSaturated, FActorPriority*& OutPriorityList, FActorPriority**& OutPriorityActors);
	int32 ServerReplicateActors_ProcessPrioritizedActors(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers, FActorPriority** PriorityActors, const int32 FinalSortedCount, int32& OutUpdated);
#endif

private:
	FPlayerSpawnRequestSender PlayerSpawner;

	friend class USpatialNetConnection;
};
