// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "RPCStore.h"
#include "Schema/MulticastRPCs.h"
#include "Schema/RPCPayload.h"
#include "SpatialView/SubView.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMulticastRPCService, Log, All);

class USpatialLatencyTracer;
class USpatialStaticComponentView;
class USpatialNetDriver;
struct RPCRingBuffer;

namespace SpatialGDK
{
class SPATIALGDK_API MulticastRPCService
{
public:
	MulticastRPCService(const ExtractRPCDelegate InExtractRPCCallback, const FSubView& InSubView, FRPCStore& InRPCStore);

	void Advance();

private:
	// Process relevant view delta changes.
	void EntityAdded(Worker_EntityId EntityId);
	void ComponentUpdate(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentUpdate* Update);
	void AuthorityGained(Worker_EntityId EntityId, Worker_ComponentId ComponentId);
	void AuthorityLost(Worker_EntityId EntityId, Worker_ComponentId ComponentId);

	// Maintain local state of multicast RPCs.
	void PopulateDataStore(Worker_EntityId EntityId);
	void ApplyComponentUpdate(Worker_EntityId EntityId, Schema_ComponentUpdate* Update);

	// Multicast system responses to state changes.
	void OnCheckoutMulticastRPCComponentOnEntity(Worker_EntityId EntityId);
	void OnRemoveMulticastRPCComponentForEntity(Worker_EntityId EntityId);
	void OnEndpointAuthorityGained(Worker_EntityId EntityId, Worker_ComponentId ComponentId);
	void OnEndpointAuthorityLost(Worker_EntityId EntityId, Worker_ComponentId ComponentId);

	// Calls ExtractRPCCallback for each RPC it extracts from a given component. If the callback returns false,
	// stops retrieving RPCs.
	void ExtractRPCs(Worker_EntityId EntityId);

	ExtractRPCDelegate ExtractRPCCallback;
	const FSubView* SubView;

	FRPCStore* RPCStore;

	// Deserialized state store for multicast components.
	TMap<Worker_EntityId_Key, MulticastRPCs> MulticastDataStore;

	// This is local, not written into schema.
	TMap<Worker_EntityId_Key, uint64> LastSeenMulticastRPCIds;
};

} // namespace SpatialGDK
