// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Interop/SpatialClassInfoManager.h"
#include "RPCStore.h"
#include "Schema/ClientEndpoint.h"
#include "Schema/RPCPayload.h"
#include "Schema/ServerEndpoint.h"
#include "SpatialView/SubView.h"
#include "Utils/RPCRingBuffer.h"

DECLARE_LOG_CATEGORY_EXTERN(LogClientServerRPCService, Log, All);

class USpatialLatencyTracer;
class USpatialStaticComponentView;
class USpatialNetDriver;
struct RPCRingBuffer;

namespace SpatialGDK
{
struct FRPCStore;

struct ClientServerEndpoints
{
	ClientEndpoint Client;
	ServerEndpoint Server;
};

class SPATIALGDK_API ClientServerRPCService
{
public:
	ClientServerRPCService(const ActorCanExtractRPCDelegate, const ExtractRPCDelegate InExtractRPCCallback, const FSubView& InSubView,
						   FRPCStore& InRPCStore);

	void AdvanceView();
	void ProcessChanges();

	// Public state functions for the main Spatial RPC service to expose bookkeeping around overflows and acks.
	// Could be moved into RPCStore. Note: Needs revisiting at some point, this is a strange boundary.
	bool ContainsOverflowedRPC(const EntityRPCType& EntityRPC) const;
	TMap<EntityRPCType, TArray<PendingRPCPayload>>& GetOverflowedRPCs();
	void AddOverflowedRPC(EntityRPCType EntityType, PendingRPCPayload&& Payload);
	void IncrementAckedRPCID(Worker_EntityId EntityId, ERPCType Type);
	uint64 GetAckFromView(Worker_EntityId EntityId, ERPCType Type);

private:
	void SetEntityData(Worker_EntityId EntityId);
	// Process relevant view delta changes.
	void EntityAdded(const Worker_EntityId EntityId);
	void ComponentUpdate(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId, Schema_ComponentUpdate* Update);

	// Maintain local state of client server RPCs.
	void PopulateDataStore(Worker_EntityId EntityId);
	void ApplyComponentUpdate(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentUpdate* Update);

	// Client server RPC system responses to state changes.
	void OnEndpointAuthorityGained(Worker_EntityId EntityId, Worker_ComponentId ComponentId);
	void OnEndpointAuthorityLost(Worker_EntityId EntityId, Worker_ComponentId ComponentId);
	void ClearOverflowedRPCs(Worker_EntityId EntityId);

	// The component with the given component ID was updated, and so there is an RPC to be handled.
	void HandleRPC(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId);
	// Calls ExtractRPCCallback for each RPC it extracts from a given component. If the callback returns false,
	// stops retrieving RPCs.
	void ExtractRPCsForEntity(Worker_EntityId EntityId, Worker_ComponentId ComponentId);
	void ExtractRPCsForType(Worker_EntityId EntityId, ERPCType Type);

	// Helpers
	const RPCRingBuffer& GetBufferFromView(Worker_EntityId EntityId, ERPCType Type);
	static bool IsClientOrServerEndpoint(Worker_ComponentId ComponentId);

	ActorCanExtractRPCDelegate CanExtractRPCDelegate;
	ExtractRPCDelegate ExtractRPCCallback;
	const FSubView* SubView;
	USpatialNetDriver* NetDriver;

	FRPCStore* RPCStore;

	// Deserialized state store for client/server RPC components.
	TMap<Worker_EntityId_Key, ClientServerEndpoints> ClientServerDataStore;

	// Stored here for things we have authority over.
	TMap<EntityRPCType, uint64> LastAckedRPCIds;
	TMap<EntityRPCType, uint64> LastSeenRPCIds;
	TMap<EntityRPCType, TArray<PendingRPCPayload>> OverflowedRPCs;
};

} // namespace SpatialGDK
