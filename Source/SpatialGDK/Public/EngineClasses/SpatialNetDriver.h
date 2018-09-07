// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CoreOnline.h"
#include "Engine.h"
#include "IpNetDriver.h"

#include "Interop/Connection/ConnectionConfig.h"
#include "Interop/SpatialOutputDevice.h"

#include <improbable/c_worker.h>

#include "SpatialNetDriver.generated.h"

class USpatialActorChannel;
class USpatialNetConnection;
class USpatialPackageMapClient;

class USpatialWorkerConnection;
class USpatialView;
class USpatialSender;
class USpatialReceiver;
class USpatialTypebindingManager;
class UGlobalStateManager;
class USpatialPlayerSpawner;

class UEntityRegistry;

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

	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar = *GLog) override;

	// Begin UNetDriver interface.
	virtual bool InitBase(bool bInitAsClient, FNetworkNotify* InNotify, const FURL& URL, bool bReuseAddressAndPort, FString& Error) override;
	virtual int32 ServerReplicateActors(float DeltaSeconds) override;
	virtual void TickDispatch(float DeltaTime) override;
	virtual void ProcessRemoteFunction(class AActor* Actor, class UFunction* Function, void* Parameters, struct FOutParmRec* OutParms, struct FFrame* NotStack, class UObject* SubObject = NULL ) override;
	virtual void TickFlush(float DeltaTime) override;
	virtual bool IsLevelInitializedForActor(const AActor* InActor, const UNetConnection* InConnection) const override;
	// End UNetDriver interface.

#if !UE_BUILD_SHIPPING
	bool HandleNetDumpCrossServerRPCCommand(const TCHAR* Cmd, FOutputDevice& Ar);
#endif

	// Returns the "100% reliable" connection to SpatialOS.
	// On the server, it is designated to be the first client connection.
	// On the client, this function is not meaningful (as we use ServerConnection)
	// Note: you should only call this after we have connected to Spatial.
	// You can check if we connected by calling GetSpatialOS()->IsConnected()
	USpatialNetConnection* GetSpatialOSNetConnection() const;

	UEntityRegistry* GetEntityRegistry() { return EntityRegistry; }

	// Used by USpatialSpawner (when new players join the game) and USpatialInteropPipelineBlock (when player controllers are migrated).
	USpatialNetConnection* AcceptNewPlayer(const FURL& InUrl, bool bExistingPlayer);

	void AddActorChannel(const Worker_EntityId& EntityId, USpatialActorChannel* Channel);
	USpatialActorChannel* GetActorChannelByEntityId(const Worker_EntityId& EntityId) const;

	UPROPERTY()
	USpatialWorkerConnection* Connection;
	UPROPERTY()
	USpatialView* View;
	UPROPERTY()
	USpatialSender* Sender;
	UPROPERTY()
	USpatialReceiver* Receiver;
	UPROPERTY()
	USpatialTypebindingManager* TypebindingManager;
	UPROPERTY()
	UGlobalStateManager* GlobalStateManager;
	UPROPERTY()
	USpatialPlayerSpawner* PlayerSpawner;
	UPROPERTY()
	USpatialPackageMapClient* PackageMap;

	TMap<UClass*, TPair<AActor*, USpatialActorChannel*>> SingletonActorChannels;

	// TODO: Remove for something better
	bool bConnectAsClient;

private:
	TUniquePtr<FSpatialOutputDevice> SpatialOutputDevice;

	UPROPERTY()
	UEntityRegistry* EntityRegistry;

	TMap<Worker_EntityId, USpatialActorChannel*> EntityToActorChannel;

	// Timer manager.
	FTimerManager* TimerManager;

	UFUNCTION()
	void OnMapLoaded(UWorld* LoadedWorld);

	void Connect();

	UFUNCTION()
	void OnConnected();

	UFUNCTION()
	void OnSpatialOSConnectFailed(const FString& Reason);

	UFUNCTION()
	void OnSpatialOSDisconnected(const FString& Reason);
		
#if WITH_SERVER_CODE
	//SpatialGDK: These functions all exist in UNetDriver, but we need to modify/simplify them in certain ways.
	// Could have marked them virtual in base class but that's a pointless source change as these functions are not meant to be called from anywhere except USpatialNetDriver::ServerReplicateActors.
	int32 ServerReplicateActors_PrepConnections(const float DeltaSeconds);
	int32 ServerReplicateActors_PrioritizeActors(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers, const TArray<FNetworkObjectInfo*> ConsiderList, const bool bCPUSaturated, FActorPriority*& OutPriorityList, FActorPriority**& OutPriorityActors);
	int32 ServerReplicateActors_ProcessPrioritizedActors(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers, FActorPriority** PriorityActors, const int32 FinalSortedCount, int32& OutUpdated);
#endif

	friend class USpatialNetConnection;
};
