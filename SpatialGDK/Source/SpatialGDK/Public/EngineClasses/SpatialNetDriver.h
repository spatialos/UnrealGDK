// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
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
class USpatialTypebindingManager;
class UGlobalStateManager;
class USpatialPlayerSpawner;
class USpatialStaticComponentView;
class USnapshotManager;

class UEntityRegistry;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialOSNetDriver, Log, All);

class FSpatialWorkerUniqueNetId : public FUniqueNetId
{
public:
	FSpatialWorkerUniqueNetId(const FString& InWorkerAttribute) : WorkerAttribute{InWorkerAttribute} {}
	~FSpatialWorkerUniqueNetId() override = default;

	const uint8* GetBytes() const override { return reinterpret_cast<const uint8*>(*WorkerAttribute); }
	int32 GetSize() const override { return WorkerAttribute.Len() * sizeof(TCHAR); }
	bool IsValid() const override { return true; }
	FString ToString() const override { return WorkerAttribute; }
	FString ToDebugString() const override { return WorkerAttribute; }
	virtual FName GetType() const override { return NULL_SUBSYSTEM; };

private:
	FString WorkerAttribute;
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
	USpatialNetConnection* AcceptNewPlayer(const FURL& InUrl, bool bExistingPlayer);

	void AddActorChannel(Worker_EntityId EntityId, USpatialActorChannel* Channel);
	void RemoveActorChannel(Worker_EntityId EntityId);

	USpatialActorChannel* GetActorChannelByEntityId(Worker_EntityId EntityId) const;

	DECLARE_DELEGATE(PostWorldWipeDelegate);

	void WipeWorld(const USpatialNetDriver::PostWorldWipeDelegate& LoadSnapshotAfterWorldWipe);

	UPROPERTY()
	USpatialWorkerConnection* Connection;
	UPROPERTY()
	USpatialDispatcher* Dispatcher;
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
	UPROPERTY()
	USpatialStaticComponentView* StaticComponentView;
	UPROPERTY()
	UEntityRegistry* EntityRegistry;
	UPROPERTY()
	USnapshotManager* SnapshotManager;

	TMap<UClass*, TPair<AActor*, USpatialActorChannel*>> SingletonActorChannels;

	bool IsAuthoritativeDestructionAllowed() const { return bAuthoritativeDestruction; }
	void StartIgnoringAuthoritativeDestruction() { bAuthoritativeDestruction = false; }
	void StopIgnoringAuthoritativeDestruction() { bAuthoritativeDestruction = true; }

#if !UE_BUILD_SHIPPING
	uint8 GetNextReliableRPCId(AActor* Actor, ESchemaComponentType RPCType, UObject* TargetObject);
	void OnReceivedReliableRPC(AActor* Actor, ESchemaComponentType RPCType, FString WorkerId, uint8 RPCId, UObject* TargetObject, UFunction* Function);

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

private:
	TUniquePtr<FSpatialOutputDevice> SpatialOutputDevice;

	TMap<Worker_EntityId_Key, USpatialActorChannel*> EntityToActorChannel;

	// Timer manager.
	FTimerManager* TimerManager;

	bool bAuthoritativeDestruction;
	bool bConnectAsClient;
	bool bPersistSpatialConnection;
	bool bWaitingForAcceptingPlayersToSpawn;
	FString SnapshotToLoad;

	UFUNCTION()
	void OnMapLoaded(UWorld* LoadedWorld);

	void Connect();

	UFUNCTION()
	void OnMapLoadedAndConnected();

	UFUNCTION()
	void OnConnectFailed(const FString& Reason);

	static void SpatialProcessServerTravel(const FString& URL, bool bAbsolute, AGameModeBase* GameMode);
		
#if WITH_SERVER_CODE
	//SpatialGDK: These functions all exist in UNetDriver, but we need to modify/simplify them in certain ways.
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
