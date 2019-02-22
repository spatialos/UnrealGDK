// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/OnlineReplStructs.h"
#include "IpNetDriver.h"
#include "OnlineSubsystemNames.h"
#include "UObject/CoreOnline.h"

#include "Interop/Connection/ConnectionConfig.h"
#include "Interop/SpatialOutputDevice.h"
#include "SpatialConstants.h"

#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialNetDriver.generated.h"

class USpatialActorChannel;
class USpatialNetConnection;
class USpatialPackageMapClient;

class USpatialWorkerConnection;
class USpatialDispatcher;
class USpatialSender;
class USpatialReceiver;
class USpatialClassInfoManager;
class UGlobalStateManager;
class USpatialPlayerSpawner;
class USpatialStaticComponentView;
class USnapshotManager;

class UEntityRegistry;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialOSNetDriver, Log, All);

DECLARE_STATS_GROUP(TEXT("SpatialNet"), STATGROUP_SpatialNet, STATCAT_Advanced);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Consider List Size"), STAT_SpatialConsiderList, STATGROUP_SpatialNet,);

DECLARE_EVENT(USpatialNetDriver, FOnConnectedEvent);
DECLARE_EVENT_OneParam(USpatialNetDriver, FOnDisconnectedEvent, const FString&);
DECLARE_EVENT_OneParam(USpatialNetDriver, FOnConnectionFailedEvent, const FString&);

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
	virtual void NotifyActorDestroyed(AActor* Actor, bool IsSeamlessTravel = false) override;
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

	// When the AcceptingPlayers state on the GSM has changed this method will be called.
	void OnAcceptingPlayersChanged(bool bAcceptingPlayers);

	// Used by USpatialSpawner (when new players join the game) and USpatialInteropPipelineBlock (when player controllers are migrated).
	USpatialNetConnection* AcceptNewPlayer(const FURL& InUrl, FUniqueNetIdRepl UniqueId, FName OnlinePlatformName, bool bExistingPlayer);

	void AddActorChannel(Worker_EntityId EntityId, USpatialActorChannel* Channel);
	void RemoveActorChannel(Worker_EntityId EntityId);

	USpatialActorChannel* GetActorChannelByEntityId(Worker_EntityId EntityId) const;

	DECLARE_DELEGATE(PostWorldWipeDelegate);

	void WipeWorld(const USpatialNetDriver::PostWorldWipeDelegate& LoadSnapshotAfterWorldWipe);

	void HandleOnConnected();
	void HandleOnDisconnected(const FString& Reason);
	void HandleOnConnectionFailed(const FString& Reason);

	// Invoked when this worker has successfully connected to SpatialOS
	FOnConnectedEvent OnConnected;
	// Invoked when this worker has disconnected from SpatialOS, both when initiated by this worker and when disconnected by the runtime
	FOnDisconnectedEvent OnDisconnected;
	// Invoked when this worker fails to initiate a connection to SpatialOS
	FOnConnectionFailedEvent OnConnectionFailed;

	UPROPERTY()
	USpatialWorkerConnection* Connection;
	UPROPERTY()
	USpatialDispatcher* Dispatcher;
	UPROPERTY()
	USpatialSender* Sender;
	UPROPERTY()
	USpatialReceiver* Receiver;
	UPROPERTY()
	USpatialClassInfoManager* ClassInfoManager;
	UPROPERTY()
	UGlobalStateManager* GlobalStateManager;
	UPROPERTY()
	USpatialPlayerSpawner* PlayerSpawner;
	UPROPERTY()
	USpatialPackageMapClient* PackageMap;
	UPROPERTY()
	USpatialStaticComponentView* StaticComponentView;
	UPROPERTY()
	UEntityRegistry* EntityRegistry;
	UPROPERTY()
	USnapshotManager* SnapshotManager;

	// Limit the number of actors which are replicated per tick to the number specified.
	// This acts as a hard limit to the number of actors per frame but nothing else. It's recommended to set this value to around 100~ (experimentation recommended).
	// If not set SpatialOS will replicate every actor per frame (unbounded) and so large worlds will experience slowdown server-side and client-side.
	// Use `stat SpatialNet` in editor builds to find the number of calls to 'ReplicateActor' and use this to inform the rate limit setting.
	UPROPERTY(Config)
	int32 ActorReplicationRateLimit;

	TMap<UClass*, TPair<AActor*, USpatialActorChannel*>> SingletonActorChannels;

	bool IsAuthoritativeDestructionAllowed() const { return bAuthoritativeDestruction; }
	void StartIgnoringAuthoritativeDestruction() { bAuthoritativeDestruction = false; }
	void StopIgnoringAuthoritativeDestruction() { bAuthoritativeDestruction = true; }

#if !UE_BUILD_SHIPPING
	uint32 GetNextReliableRPCId(AActor* Actor, ESchemaComponentType RPCType, UObject* TargetObject);
	void OnReceivedReliableRPC(AActor* Actor, ESchemaComponentType RPCType, FString WorkerId, uint32 RPCId, UObject* TargetObject, UFunction* Function);
	void OnRPCAuthorityGained(AActor* Actor, ESchemaComponentType RPCType);

	struct FReliableRPCId
	{
		FReliableRPCId() = default;
		FReliableRPCId(FString InWorkerId, uint32 InRPCId, FString InRPCTarget, FString InRPCName) : WorkerId(InWorkerId), RPCId(InRPCId), LastRPCTarget(InRPCTarget), LastRPCName(InRPCName) {}

		FString WorkerId;
		uint32 RPCId = 0;
		FString LastRPCTarget;
		FString LastRPCName;
	};
	using FRPCTypeToReliableRPCIdMap = TMap<ESchemaComponentType, FReliableRPCId>;
	// Per actor, maps from RPC type to the reliable RPC index used to detect if reliable RPCs go out of order.
	TMap<TWeakObjectPtr<AActor>, FRPCTypeToReliableRPCIdMap> ReliableRPCIdMap;
#endif // !UE_BUILD_SHIPPING

	void DelayedSendDeleteEntityRequest(Worker_EntityId EntityId, float Delay);

private:
	TUniquePtr<FSpatialOutputDevice> SpatialOutputDevice;

	TMap<Worker_EntityId_Key, USpatialActorChannel*> EntityToActorChannel;

	FTimerManager* TimerManager;

	bool bAuthoritativeDestruction;
	bool bConnectAsClient;
	bool bPersistSpatialConnection;
	bool bWaitingForAcceptingPlayersToSpawn;
	FString SnapshotToLoad;

	UFUNCTION()
	void OnMapLoaded(UWorld* LoadedWorld);

	UFUNCTION()
	void OnLevelAddedToWorld(ULevel* LoadedLevel, UWorld* OwningWorld);

	void Connect();

	UFUNCTION()
	void OnMapLoadedAndConnected();

	static void SpatialProcessServerTravel(const FString& URL, bool bAbsolute, AGameModeBase* GameMode);

#if WITH_SERVER_CODE
	// SpatialGDK: These functions all exist in UNetDriver, but we need to modify/simplify them in certain ways.
	// Could have marked them virtual in base class but that's a pointless source change as these functions are not meant to be called from anywhere except USpatialNetDriver::ServerReplicateActors.
	int32 ServerReplicateActors_PrepConnections(const float DeltaSeconds);
	int32 ServerReplicateActors_PrioritizeActors(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers, const TArray<FNetworkObjectInfo*> ConsiderList, const bool bCPUSaturated, FActorPriority*& OutPriorityList, FActorPriority**& OutPriorityActors);
	int32 ServerReplicateActors_ProcessPrioritizedActors(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers, FActorPriority** PriorityActors, const int32 FinalSortedCount, int32& OutUpdated);
#endif

	friend class USpatialNetConnection;

	// This index is incremented and assigned to every new RPC in ProcessRemoteFunction.
	// The SpatialSender uses these indexes to retry any failed reliable RPCs
	// in the correct order, if needed.
	int NextRPCIndex;
};
