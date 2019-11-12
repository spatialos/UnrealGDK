// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/OnlineReplStructs.h"
#include "IpNetDriver.h"
#include "OnlineSubsystemNames.h"
#include "TimerManager.h"
#include "UObject/CoreOnline.h"

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/Connection/ConnectionConfig.h"
#include "Interop/SpatialOutputDevice.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"

#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialNetDriver.generated.h"

class ASpatialMetricsDisplay;
class UAbstractLBStrategy;
class UActorGroupManager;
class UEntityPool;
class UGlobalStateManager;
class USnapshotManager;
class USpatialActorChannel;
class USpatialClassInfoManager;
class USpatialDispatcher;
class USpatialLoadBalanceEnforcer;
class USpatialMetrics;
class USpatialNetConnection;
class USpatialPackageMapClient;
class USpatialPlayerSpawner;
class USpatialReceiver;
class USpatialSender;
class USpatialStaticComponentView;
class USpatialWorkerConnection;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialOSNetDriver, Log, All);

DECLARE_STATS_GROUP(TEXT("SpatialNet"), STATGROUP_SpatialNet, STATCAT_Advanced);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Consider List Size"), STAT_SpatialConsiderList, STATGROUP_SpatialNet,);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Num Relevant Actors"), STAT_SpatialActorsRelevant, STATGROUP_SpatialNet,);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Num Changed Relevant Actors"), STAT_SpatialActorsChanged, STATGROUP_SpatialNet,);

UCLASS()
class SPATIALGDK_API USpatialNetDriver : public UIpNetDriver
{
	GENERATED_BODY()

public:

	USpatialNetDriver(const FObjectInitializer& ObjectInitializer);

	// Begin UObject Interface
	virtual void BeginDestroy() override;
	virtual void PostInitProperties() override;
	// End UObject Interface

	// Begin FExec Interface
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar = *GLog) override;
	// End FExec Interface

	// Begin UNetDriver interface.
	virtual bool InitBase(bool bInitAsClient, FNetworkNotify* InNotify, const FURL& URL, bool bReuseAddressAndPort, FString& Error) override;
	virtual int32 ServerReplicateActors(float DeltaSeconds) override;
	virtual void TickDispatch(float DeltaTime) override;
	virtual void ProcessRemoteFunction(class AActor* Actor, class UFunction* Function, void* Parameters, struct FOutParmRec* OutParms, struct FFrame* NotStack, class UObject* SubObject = NULL ) override;
	virtual void TickFlush(float DeltaTime) override;
	virtual bool IsLevelInitializedForActor(const AActor* InActor, const UNetConnection* InConnection) const override;
	virtual void NotifyActorDestroyed(AActor* Actor, bool IsSeamlessTravel = false) override;
	virtual void Shutdown() override;
	virtual void NotifyActorFullyDormantForConnection(AActor* Actor, UNetConnection* NetConnection) override;
	// End UNetDriver interface.

	virtual void OnOwnerUpdated(AActor* Actor);

	void OnConnectedToSpatialOS();

#if !UE_BUILD_SHIPPING
	bool HandleNetDumpCrossServerRPCCommand(const TCHAR* Cmd, FOutputDevice& Ar);
#endif

	// Returns the "100% reliable" connection to SpatialOS.
	// On the server, it is designated to be the first client connection.
	// On the client, this function is not meaningful (as we use ServerConnection)
	// Note: you should only call this after we have connected to Spatial.
	// You can check if we connected by calling GetSpatialOS()->IsConnected()
	USpatialNetConnection* GetSpatialOSNetConnection() const;

	// When the AcceptingPlayers state on the GSM has changed this method will be called.
	void OnAcceptingPlayersChanged(bool bAcceptingPlayers);

	// Used by USpatialSpawner (when new players join the game) and USpatialInteropPipelineBlock (when player controllers are migrated).
	void AcceptNewPlayer(const FURL& InUrl, const FUniqueNetIdRepl& UniqueId, const FName& OnlinePlatformName);
	void PostSpawnPlayerController(APlayerController* PlayerController, const FString& WorkerAttribute);

	void AddActorChannel(Worker_EntityId EntityId, USpatialActorChannel* Channel);
	void RemoveActorChannel(Worker_EntityId EntityId);
	TMap<Worker_EntityId_Key, USpatialActorChannel*>& GetEntityToActorChannelMap();

	USpatialActorChannel* GetOrCreateSpatialActorChannel(UObject* TargetObject);
	USpatialActorChannel* GetActorChannelByEntityId(Worker_EntityId EntityId) const;

	void RefreshActorDormancy(AActor* Actor, bool bMakeDormant);

	void AddPendingDormantChannel(USpatialActorChannel* Channel);
	void RegisterDormantEntityId(Worker_EntityId EntityId);
	void UnregisterDormantEntityId(Worker_EntityId EntityId);
	bool IsDormantEntity(Worker_EntityId EntityId) const;

	DECLARE_DELEGATE(PostWorldWipeDelegate);

	void WipeWorld(const USpatialNetDriver::PostWorldWipeDelegate& LoadSnapshotAfterWorldWipe);

	void SetSpatialMetricsDisplay(ASpatialMetricsDisplay* InSpatialMetricsDisplay);

	UPROPERTY()
	USpatialWorkerConnection* Connection;
	UPROPERTY()
	USpatialDispatcher* Dispatcher;
	UPROPERTY()
	USpatialSender* Sender;
	UPROPERTY()
	USpatialReceiver* Receiver;
	UPROPERTY()
	UActorGroupManager* ActorGroupManager;
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
	USnapshotManager* SnapshotManager;
	UPROPERTY()
	USpatialMetrics* SpatialMetrics;
	UPROPERTY()
	ASpatialMetricsDisplay* SpatialMetricsDisplay;
	UPROPERTY()
	USpatialLoadBalanceEnforcer* LoadBalanceEnforcer;
	UPROPERTY()
	UAbstractLBStrategy* LoadBalanceStrategy;

	Worker_EntityId WorkerEntityId = SpatialConstants::INVALID_ENTITY_ID;

	TMap<UClass*, TPair<AActor*, USpatialActorChannel*>> SingletonActorChannels;

	bool IsAuthoritativeDestructionAllowed() const { return bAuthoritativeDestruction; }
	void StartIgnoringAuthoritativeDestruction() { bAuthoritativeDestruction = false; }
	void StopIgnoringAuthoritativeDestruction() { bAuthoritativeDestruction = true; }

#if !UE_BUILD_SHIPPING
	int32 GetConsiderListSize() const { return ConsiderListSize; }
#endif

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

	void DelayedSendDeleteEntityRequest(Worker_EntityId EntityId, float Delay);

#if WITH_EDITOR
	// We store the PlayInEditorID associated with this NetDriver to handle replace a worker initialization when in the editor.
	int32 PlayInEditorID;

	void TrackTombstone(const Worker_EntityId EntityId);
#endif

private:
	TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator;
	TUniquePtr<FSpatialOutputDevice> SpatialOutputDevice;

	TMap<Worker_EntityId_Key, USpatialActorChannel*> EntityToActorChannel;
	TArray<Worker_OpList*> QueuedStartupOpLists;
	TSet<Worker_EntityId_Key> DormantEntities;
	TSet<TWeakObjectPtr<USpatialActorChannel>> PendingDormantChannels;

	FTimerManager TimerManager;

	bool bAuthoritativeDestruction;
	bool bConnectAsClient;
	bool bPersistSpatialConnection;
	bool bWaitingForAcceptingPlayersToSpawn;
	bool bIsReadyToStart;
	bool bMapLoaded;

	FString SnapshotToLoad;

	class USpatialGameInstance* GetGameInstance() const;

	void InitiateConnectionToSpatialOS(const FURL& URL);

	void InitializeSpatialOutputDevice();
	void CreateAndInitializeCoreClasses();

	void CreateServerSpatialOSNetConnection();
	USpatialActorChannel* CreateSpatialActorChannel(AActor* Actor);

	void QueryGSMToLoadMap();

	void HandleOngoingServerTravel();

	void HandleStartupOpQueueing(const TArray<Worker_OpList*>& InOpLists);
	bool FindAndDispatchStartupOpsServer(const TArray<Worker_OpList*>& InOpLists);
	bool FindAndDispatchStartupOpsClient(const TArray<Worker_OpList*>& InOpLists);
	void SelectiveProcessOps(TArray<Worker_Op*> FoundOps);

	UFUNCTION()
	void OnMapLoaded(UWorld* LoadedWorld);

	UFUNCTION()
	void OnLevelAddedToWorld(ULevel* LoadedLevel, UWorld* OwningWorld);

	static void SpatialProcessServerTravel(const FString& URL, bool bAbsolute, AGameModeBase* GameMode);

#if WITH_SERVER_CODE
	// SpatialGDK: These functions all exist in UNetDriver, but we need to modify/simplify them in certain ways.
	// Could have marked them virtual in base class but that's a pointless source change as these functions are not meant to be called from anywhere except USpatialNetDriver::ServerReplicateActors.
	int32 ServerReplicateActors_PrepConnections(const float DeltaSeconds);
	int32 ServerReplicateActors_PrioritizeActors(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers, const TArray<FNetworkObjectInfo*> ConsiderList, const bool bCPUSaturated, FActorPriority*& OutPriorityList, FActorPriority**& OutPriorityActors);
	void ServerReplicateActors_ProcessPrioritizedActors(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers, FActorPriority** PriorityActors, const int32 FinalSortedCount, int32& OutUpdated);
#endif

	void ProcessRPC(AActor* Actor, UObject* SubObject, UFunction* Function, void* Parameters);
	bool CreateSpatialNetConnection(const FURL& InUrl, const FUniqueNetIdRepl& UniqueId, const FName& OnlinePlatformName, USpatialNetConnection** OutConn);

	void ProcessPendingDormancy();

	friend USpatialNetConnection;
	friend USpatialWorkerConnection;

	// This index is incremented and assigned to every new RPC in ProcessRemoteFunction.
	// The SpatialSender uses these indexes to retry any failed reliable RPCs
	// in the correct order, if needed.
	int NextRPCIndex;

	float TimeWhenPositionLastUpdated;

	// Counter for giving each connected client a unique IP address to satisfy Unreal's requirement of
	// each client having a unique IP address in the UNetDriver::MappedClientConnections map.
	// The GDK does not use this address for any networked purpose, only bookkeeping.
	uint32 UniqueClientIpAddressCounter = 0;

	FDelegateHandle SpatialDeploymentStartHandle;

#if !UE_BUILD_SHIPPING
	int32 ConsiderListSize = 0;
#endif

#if WITH_EDITOR
	static const int32 EDITOR_TOMBSTONED_ENTITY_TRACKING_RESERVATION_COUNT = 256;
	TArray<Worker_EntityId> TombstonedEntities;
#endif
};
