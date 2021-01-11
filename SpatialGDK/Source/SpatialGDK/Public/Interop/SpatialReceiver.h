// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/RPCs/SpatialRPCService.h"
#include "Interop/SpatialClassInfoManager.h"
#include "Interop/SpatialOSDispatcherInterface.h"
#include "Schema/DynamicComponent.h"
#include "Schema/NetOwningClientWorker.h"
#include "Schema/RPCPayload.h"
#include "Schema/SpawnData.h"
#include "Schema/UnrealObjectRef.h"
#include "SpatialCommonTypes.h"
#include "SpatialView/OpList/EntityComponentOpList.h"
#include "Utils/GDKPropertyMacros.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialReceiver.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialReceiver, Log, All);

class USpatialNetConnection;
class USpatialSender;
class UGlobalStateManager;
class SpatialLoadBalanceEnforcer;

namespace SpatialGDK
{
class SpatialEventTracer;
} // namespace SpatialGDK

struct PendingAddComponentWrapper
{
	PendingAddComponentWrapper() = default;
	PendingAddComponentWrapper(Worker_EntityId InEntityId, Worker_ComponentId InComponentId,
							   TUniquePtr<SpatialGDK::DynamicComponent>&& InData)
		: EntityId(InEntityId)
		, ComponentId(InComponentId)
		, Data(MoveTemp(InData))
	{
	}

	// We define equality to cover just entity and component IDs since duplicated AddComponent ops
	// will be moved into unique pointers and we cannot equate the underlying Worker_ComponentData.
	bool operator==(const PendingAddComponentWrapper& Other) const
	{
		return EntityId == Other.EntityId && ComponentId == Other.ComponentId;
	}

	Worker_EntityId EntityId;
	Worker_ComponentId ComponentId;
	TUniquePtr<SpatialGDK::DynamicComponent> Data;
};

UCLASS()
class USpatialReceiver : public UObject, public SpatialOSDispatcherInterface
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* NetDriver, FTimerManager* InTimerManager, SpatialGDK::SpatialEventTracer* InEventTracer);

	// Dispatcher Calls
	virtual void OnCommandRequest(const Worker_Op& Op) override;
	virtual void OnCommandResponse(const Worker_Op& Op) override;

	virtual void OnReserveEntityIdsResponse(const Worker_ReserveEntityIdsResponseOp& Op) override;
	virtual void OnCreateEntityResponse(const Worker_Op& Op) override;

	virtual void AddPendingActorRequest(Worker_RequestId RequestId, USpatialActorChannel* Channel) override;
	virtual void AddPendingReliableRPC(Worker_RequestId RequestId, TSharedRef<struct FReliableRPCForRetry> ReliableRPC) override;

	virtual void AddEntityQueryDelegate(Worker_RequestId RequestId, EntityQueryDelegate Delegate) override;
	virtual void AddReserveEntityIdsDelegate(Worker_RequestId RequestId, ReserveEntityIDsDelegate Delegate) override;
	virtual void AddCreateEntityDelegate(Worker_RequestId RequestId, CreateEntityDelegate Delegate) override;
	virtual void AddSystemEntityCommandDelegate(Worker_RequestId RequestId, SystemEntityCommandDelegate Delegate) override;

	virtual void OnEntityQueryResponse(const Worker_EntityQueryResponseOp& Op) override;

	virtual void OnSystemEntityCommandResponse(const Worker_CommandResponseOp& Op) override;

	void OnDisconnect(uint8 StatusCode, const FString& Reason);

	bool IsPendingOpsOnChannel(USpatialActorChannel& Channel);

private:
	TWeakObjectPtr<USpatialActorChannel> PopPendingActorRequest(Worker_RequestId RequestId);

	void CloseClientConnection(USpatialNetConnection* ClientConnection, Worker_EntityId PlayerControllerEntityId);

	void ReceiveWorkerDisconnectResponse(const Worker_CommandResponseOp& Op);
	void ReceiveClaimPartitionResponse(const Worker_CommandResponseOp& Op);

public:
	FOnEntityAddedDelegate OnEntityAddedDelegate;
	FOnEntityRemovedDelegate OnEntityRemovedDelegate;

	TMap<Worker_RequestId_Key, Worker_PartitionId> PendingPartitionAssignments;

private:
	UPROPERTY()
	USpatialNetDriver* NetDriver;

	UPROPERTY()
	USpatialStaticComponentView* StaticComponentView;

	UPROPERTY()
	USpatialSender* Sender;

	UPROPERTY()
	USpatialPackageMapClient* PackageMap;

	UPROPERTY()
	USpatialClassInfoManager* ClassInfoManager;

	UPROPERTY()
	UGlobalStateManager* GlobalStateManager;

	FTimerManager* TimerManager;

	// Helper struct to manage FSpatialObjectRepState update cycle.
	struct RepStateUpdateHelper;

	bool bInCriticalSection;
	TArray<Worker_EntityId> PendingAddActors;
	TArray<Worker_ComponentSetAuthorityChangeOp> PendingAuthorityChanges;
	TArray<PendingAddComponentWrapper> PendingAddComponents;
	TArray<Worker_RemoveComponentOp> QueuedRemoveComponentOps;

	TMap<Worker_RequestId_Key, TWeakObjectPtr<USpatialActorChannel>> PendingActorRequests;
	FReliableRPCMap PendingReliableRPCs;

	TMap<Worker_RequestId_Key, EntityQueryDelegate> EntityQueryDelegates;
	TMap<Worker_RequestId_Key, ReserveEntityIDsDelegate> ReserveEntityIDsDelegates;
	TMap<Worker_RequestId_Key, CreateEntityDelegate> CreateEntityDelegates;
	TMap<Worker_RequestId_Key, SystemEntityCommandDelegate> SystemEntityCommandDelegates;

	// This will map PlayerController entities to the corresponding SpatialNetConnection
	// for PlayerControllers that this server has authority over. This is used for player
	// lifecycle logic (Heartbeat component updates, disconnection logic).
	TMap<Worker_EntityId_Key, TWeakObjectPtr<USpatialNetConnection>> AuthorityPlayerControllerConnectionMap;

	TMap<TPair<Worker_EntityId_Key, Worker_ComponentId>, PendingAddComponentWrapper> PendingDynamicSubobjectComponents;
	TMap<Worker_EntityId_Key, FString> WorkerConnectionEntities;

	SpatialGDK::SpatialEventTracer* EventTracer;
};
