// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/SpatialLoadBalanceEnforcer.h"
#include "EngineClasses/SpatialVirtualWorkerTranslationManager.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/Connection/ConnectionConfig.h"
#include "Interop/SpatialDispatcher.h"
#include "Interop/SpatialOutputDevice.h"
#include "Interop/SpatialRPCService.h"
#include "Interop/SpatialSnapshotManager.h"
#include "SpatialView/OpList/OpList.h"
#include "Utils/InterestFactory.h"

#include "LoadBalancing/AbstractLockingPolicy.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"

#include "CoreMinimal.h"
#include "GameFramework/OnlineReplStructs.h"
#include "IpNetDriver.h"
#include "TimerManager.h"

#include "SpatialNetDriver.generated.h"

class ASpatialDebugger;
class ASpatialMetricsDisplay;
class FSpatialLoadBalancingHandler;
class UAbstractLBStrategy;
class UEntityPool;
class UGlobalStateManager;
class USpatialActorChannel;
class USpatialClassInfoManager;
class USpatialConnectionManager;
class USpatialGameInstance;
class USpatialMetrics;
class USpatialNetConnection;
class USpatialPackageMapClient;
class USpatialPlayerSpawner;
class USpatialReceiver;
class USpatialSender;
class USpatialStaticComponentView;
class USpatialWorkerConnection;
class USpatialWorkerFlags;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialOSNetDriver, Log, All);

DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Consider List Size"), STAT_SpatialConsiderList, STATGROUP_SpatialNet, );
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Num Relevant Actors"), STAT_SpatialActorsRelevant, STATGROUP_SpatialNet, );
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Num Changed Relevant Actors"), STAT_SpatialActorsChanged, STATGROUP_SpatialNet, );

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
	virtual bool InitBase(bool bInitAsClient, FNetworkNotify* InNotify, const FURL& URL, bool bReuseAddressAndPort,
						  FString& Error) override;
	virtual int32 ServerReplicateActors(float DeltaSeconds) override;
	virtual void TickDispatch(float DeltaTime) override;
	virtual void ProcessRemoteFunction(class AActor* Actor, class UFunction* Function, void* Parameters, struct FOutParmRec* OutParms,
									   struct FFrame* NotStack, class UObject* SubObject = NULL) override;
	virtual void TickFlush(float DeltaTime) override;
	virtual bool IsLevelInitializedForActor(const AActor* InActor, const UNetConnection* InConnection) const override;
	virtual void NotifyActorDestroyed(AActor* Actor, bool IsSeamlessTravel = false) override;
	virtual void Shutdown() override;
	virtual void NotifyActorFullyDormantForConnection(AActor* Actor, UNetConnection* NetConnection) override;
	virtual void OnOwnerUpdated(AActor* Actor, AActor* OldOwner) override;
	// End UNetDriver interface.

	void OnConnectionToSpatialOSSucceeded();
	void OnConnectionToSpatialOSFailed(uint8_t ConnectionStatusCode, const FString& ErrorMessage);

#if !UE_BUILD_SHIPPING
	bool HandleNetDumpCrossServerRPCCommand(const TCHAR* Cmd, FOutputDevice& Ar);
#endif

	// Returns the "100% reliable" connection to SpatialOS.
	// On the server, it is designated to be the first client connection.
	// On the client, this function is not meaningful (as we use ServerConnection)
	// Note: you should only call this after we have connected to Spatial.
	// You can check if we connected by calling GetSpatialOS()->IsConnected()
	USpatialNetConnection* GetSpatialOSNetConnection() const;

	// When the AcceptingPlayers/SessionID state on the GSM has changed this method will be called.
	void OnGSMQuerySuccess();
	void RetryQueryGSM();
	void GSMQueryDelegateFunction(const Worker_EntityQueryResponseOp& Op);

	// Used by USpatialSpawner (when new players join the game) and USpatialInteropPipelineBlock (when player controllers are migrated).
	void AcceptNewPlayer(const FURL& InUrl, const FUniqueNetIdRepl& UniqueId, const FName& OnlinePlatformName);
	void PostSpawnPlayerController(APlayerController* PlayerController, const FString& ConnectionOwningWorkerId);

	void AddActorChannel(Worker_EntityId EntityId, USpatialActorChannel* Channel);
	void RemoveActorChannel(Worker_EntityId EntityId, USpatialActorChannel& Channel);
	TMap<Worker_EntityId_Key, USpatialActorChannel*>& GetEntityToActorChannelMap();

	USpatialActorChannel* GetOrCreateSpatialActorChannel(UObject* TargetObject);
	USpatialActorChannel* GetActorChannelByEntityId(Worker_EntityId EntityId) const;

	void RefreshActorDormancy(AActor* Actor, bool bMakeDormant);

	void AddPendingDormantChannel(USpatialActorChannel* Channel);
	void RemovePendingDormantChannel(USpatialActorChannel* Channel);
	void RegisterDormantEntityId(Worker_EntityId EntityId);
	void UnregisterDormantEntityId(Worker_EntityId EntityId);
	bool IsDormantEntity(Worker_EntityId EntityId) const;

	void WipeWorld(const PostWorldWipeDelegate& LoadSnapshotAfterWorldWipe);

	void SetSpatialMetricsDisplay(ASpatialMetricsDisplay* InSpatialMetricsDisplay);
	void SetSpatialDebugger(ASpatialDebugger* InSpatialDebugger);
	TWeakObjectPtr<USpatialNetConnection> FindClientConnectionFromWorkerId(const FString& WorkerId);
	void CleanUpClientConnection(USpatialNetConnection* ClientConnection);

	UPROPERTY()
	USpatialWorkerConnection* Connection;
	UPROPERTY()
	USpatialConnectionManager* ConnectionManager;
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
	USpatialMetrics* SpatialMetrics;
	UPROPERTY()
	ASpatialMetricsDisplay* SpatialMetricsDisplay;
	UPROPERTY()
	ASpatialDebugger* SpatialDebugger;
	UPROPERTY()
	UAbstractLBStrategy* LoadBalanceStrategy;
	UPROPERTY()
	UAbstractLockingPolicy* LockingPolicy;
	UPROPERTY()
	USpatialWorkerFlags* SpatialWorkerFlags;

	TUniquePtr<SpatialGDK::InterestFactory> InterestFactory;
	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer;
	TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator;

	Worker_EntityId WorkerEntityId = SpatialConstants::INVALID_ENTITY_ID;

	// If this worker is authoritative over the translation, the manager will be instantiated.
	TUniquePtr<SpatialVirtualWorkerTranslationManager> VirtualWorkerTranslationManager;
	void InitializeVirtualWorkerTranslationManager();

	bool IsAuthoritativeDestructionAllowed() const { return bAuthoritativeDestruction; }
	void StartIgnoringAuthoritativeDestruction() { bAuthoritativeDestruction = false; }
	void StopIgnoringAuthoritativeDestruction() { bAuthoritativeDestruction = true; }

#if !UE_BUILD_SHIPPING
	int32 GetConsiderListSize() const { return ConsiderListSize; }
#endif

	void DelayedRetireEntity(Worker_EntityId EntityId, float Delay, bool bIsNetStartupActor);

#if WITH_EDITOR
	// We store the PlayInEditorID associated with this NetDriver to handle replace a worker initialization when in the editor.
	int32 PlayInEditorID;

	void TrackTombstone(const Worker_EntityId EntityId);
#endif

	// IsReady evaluates the GSM, Load Balancing system, and others to get a holistic
	// view of whether the SpatialNetDriver is ready to assume normal operations.
	bool IsReady() const;

	SpatialGDK::SpatialRPCService* GetRPCService() const { return RPCService.Get(); }

#if ENGINE_MINOR_VERSION <= 24
	float GetElapsedTime() { return Time; }
#endif

private:
	TUniquePtr<SpatialDispatcher> Dispatcher;
	TUniquePtr<SpatialSnapshotManager> SnapshotManager;
	TUniquePtr<FSpatialOutputDevice> SpatialOutputDevice;

	TUniquePtr<SpatialGDK::SpatialRPCService> RPCService;

	TMap<Worker_EntityId_Key, USpatialActorChannel*> EntityToActorChannel;
	TArray<SpatialGDK::OpList> QueuedStartupOpLists;
	TSet<Worker_EntityId_Key> DormantEntities;
	TSet<TWeakObjectPtr<USpatialActorChannel>> PendingDormantChannels;

	TMap<FString, TWeakObjectPtr<USpatialNetConnection>> WorkerConnections;

	FTimerManager TimerManager;

	bool bAuthoritativeDestruction;
	bool bConnectAsClient;
	bool bPersistSpatialConnection;
	bool bWaitingToSpawn;
	bool bIsReadyToStart;
	bool bMapLoaded;

	FString SnapshotToLoad;

	// Client variable which stores the SessionId given to us by the server in the URL options.
	// Used to compare against the GSM SessionId to ensure the the server is ready to spawn players.
	int32 SessionId;

	class USpatialGameInstance* GetGameInstance() const;

	void InitiateConnectionToSpatialOS(const FURL& URL);

	void InitializeSpatialOutputDevice();
	void CreateAndInitializeCoreClasses();
	void CreateAndInitializeLoadBalancingClasses();

	void CreateServerSpatialOSNetConnection();
	USpatialActorChannel* CreateSpatialActorChannel(AActor* Actor);

	void QueryGSMToLoadMap();

	void HandleStartupOpQueueing(TArray<SpatialGDK::OpList> InOpLists);
	bool FindAndDispatchStartupOpsServer(const TArray<SpatialGDK::OpList>& InOpLists);
	bool FindAndDispatchStartupOpsClient(const TArray<SpatialGDK::OpList>& InOpLists);
	void SelectiveProcessOps(TArray<Worker_Op*> FoundOps);

	UFUNCTION()
	void OnMapLoaded(UWorld* LoadedWorld);

	UFUNCTION()
	void OnLevelAddedToWorld(ULevel* LoadedLevel, UWorld* OwningWorld);

	void OnActorSpawned(AActor* Actor);

	static void SpatialProcessServerTravel(const FString& URL, bool bAbsolute, AGameModeBase* GameMode);

#if WITH_SERVER_CODE
	// SpatialGDK: These functions all exist in UNetDriver, but we need to modify/simplify them in certain ways.
	// Could have marked them virtual in base class but that's a pointless source change as these functions are not meant to be called from
	// anywhere except USpatialNetDriver::ServerReplicateActors.
	int32 ServerReplicateActors_PrepConnections(const float DeltaSeconds);
	int32 ServerReplicateActors_PrioritizeActors(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers,
												 FSpatialLoadBalancingHandler&, const TArray<FNetworkObjectInfo*> ConsiderList,
												 const bool bCPUSaturated, FActorPriority*& OutPriorityList,
												 FActorPriority**& OutPriorityActors);
	void ServerReplicateActors_ProcessPrioritizedActors(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers,
														FSpatialLoadBalancingHandler&, FActorPriority** PriorityActors,
														const int32 FinalSortedCount, int32& OutUpdated);
#endif

	void ProcessRPC(AActor* Actor, UObject* SubObject, UFunction* Function, void* Parameters);
	bool CreateSpatialNetConnection(const FURL& InUrl, const FUniqueNetIdRepl& UniqueId, const FName& OnlinePlatformName,
									USpatialNetConnection** OutConn);

	void ProcessPendingDormancy();
	void PollPendingLoads();

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

	void MakePlayerSpawnRequest();

	FUnrealObjectRef GetCurrentPlayerControllerRef();

	// Checks the GSM is acceptingPlayers and that the SessionId on the GSM matches the SessionId on the net-driver.
	// The SessionId on the net-driver is set by looking at the sessionId option in the URL sent to the client for ServerTravel.
	bool ClientCanSendPlayerSpawnRequests();
};
